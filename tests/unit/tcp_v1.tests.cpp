#include "matching_engine/command/limit_command.h"
#include "matching_engine/event/ack_event.h"
#include "matching_engine/event/fill_event.h"
#include "protocol/codec.h"
#include "protocol/framer.h"
#include "server/connection.h"

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

int g_total_tests = 0;
int g_failed_tests = 0;
int g_total_checks = 0;
int g_failed_checks = 0;
const char* g_current_test = "";

void check(bool condition, const char* expr, const char* file, int line) {
    ++g_total_checks;
    if (!condition) {
        ++g_failed_checks;
        std::cerr << "[FAIL] " << g_current_test << ": " << expr
                  << " (" << file << ":" << line << ")\n";
    }
}

template <typename A, typename B>
void check_eq(const A& a, const B& b, const char* expr_a, const char* expr_b, const char* file, int line) {
    ++g_total_checks;
    bool equal = false;
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        equal = std::cmp_equal(a, b);
    } else {
        equal = (a == b);
    }
    if (!equal) {
        ++g_failed_checks;
        std::cerr << "[FAIL] " << g_current_test << ": " << expr_a << " == " << expr_b
                  << " (got " << a << " vs " << b << ")"
                  << " (" << file << ":" << line << ")\n";
    }
}

#define CHECK(expr) check((expr), #expr, __FILE__, __LINE__)
#define CHECK_EQ(a, b) check_eq((a), (b), #a, #b, __FILE__, __LINE__)

void run_test(const char* name, void (*fn)()) {
    ++g_total_tests;
    g_current_test = name;
    const int failed_before = g_failed_checks;
    try {
        fn();
    } catch (const std::exception& e) {
        const std::string what = e.what();
        if (what.find("Operation not permitted") != std::string::npos) {
            std::cout << "[SKIP] " << name << " (" << what << ")\n";
            return;
        }
        ++g_failed_checks;
        std::cerr << "[FAIL] " << name << ": unexpected exception: " << e.what() << "\n";
    } catch (...) {
        ++g_failed_checks;
        std::cerr << "[FAIL] " << name << ": unexpected non-std exception\n";
    }
    if (g_failed_checks == failed_before) {
        std::cout << "[PASS] " << name << "\n";
    } else {
        ++g_failed_tests;
        std::cout << "[FAIL] " << name << "\n";
    }
}

int count_type(const std::vector<std::unique_ptr<Event>>& events, EventType type) {
    int count = 0;
    for (const auto& event : events) {
        if (event->getType() == type) {
            ++count;
        }
    }
    return count;
}

const FillEvent* find_fill_for(const std::vector<std::unique_ptr<Event>>& events, uint64_t order_id) {
    for (const auto& event : events) {
        if (event->getType() != EventType::Fill) {
            continue;
        }
        const auto* fill = static_cast<const FillEvent*>(event.get());
        if (fill->getOrderId() == order_id) {
            return fill;
        }
    }
    return nullptr;
}

class TcpV1Harness {
public:
    TcpV1Harness()
        : _matching_engine("ACME"),
          _acceptor(_server_io),
          _client_socket(_client_io) {
        boost::system::error_code ec;
        _acceptor.open(boost::asio::ip::tcp::v4(), ec);
        if (ec) {
            _init_error = "acceptor open failed: " + ec.message();
            return;
        }

        _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if (ec) {
            _init_error = "acceptor set_option failed: " + ec.message();
            return;
        }

        _acceptor.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 0), ec);
        if (ec) {
            _init_error = "acceptor bind failed: " + ec.message();
            return;
        }

        _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) {
            _init_error = "acceptor listen failed: " + ec.message();
            return;
        }

        _connection = Connection::create(_server_io, _server_codec, _matching_engine);

        _acceptor.async_accept(_connection->socket(), [this](const boost::system::error_code& ec) {
            {
                std::lock_guard<std::mutex> lock(_mx);
                _accept_done = true;
                _accept_ok = !ec;
            }
            if (!ec) {
                _connection->start();
            }
            _cv.notify_one();
        });

        _server_thread = std::thread([this]() { _server_io.run(); });

        const auto endpoint = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address_v4::loopback(),
            _acceptor.local_endpoint().port());
        _client_socket.connect(endpoint, ec);
        if (ec) {
            _init_error = "client connect failed: " + ec.message();
            return;
        }

        _client_socket.non_blocking(true, ec);
        if (!ec) {
            std::unique_lock<std::mutex> lock(_mx);
            _cv.wait_for(lock, std::chrono::seconds(1), [this]() { return _accept_done; });
            if (!_accept_done || !_accept_ok) {
                _init_error = "accept did not complete";
                return;
            }

            _ready = true;
            return;
        }
        _init_error = "client non_blocking failed: " + ec.message();
    }

    ~TcpV1Harness() {
        shutdown();
    }

    bool ready() const {
        return _ready;
    }

    const std::string& init_error() const {
        return _init_error;
    }

    bool send_command(const Command& cmd) {
        auto payload = _client_codec.encodeCommand(cmd);
        auto framed = _client_framer.frame(payload);
        return send_raw(framed);
    }

    bool send_fragmented_command(const Command& cmd, std::size_t cut_pos) {
        auto payload = _client_codec.encodeCommand(cmd);
        auto framed = _client_framer.frame(payload);
        if (cut_pos >= framed.size()) {
            return false;
        }

        std::vector<uint8_t> part1(framed.begin(), framed.begin() + static_cast<std::ptrdiff_t>(cut_pos));
        std::vector<uint8_t> part2(framed.begin() + static_cast<std::ptrdiff_t>(cut_pos), framed.end());
        if (!send_raw(part1)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return send_raw(part2);
    }

    bool send_raw(const std::vector<uint8_t>& bytes) {
        if (!_client_socket.is_open()) {
            return false;
        }

        boost::system::error_code ec;
        _client_socket.non_blocking(false, ec);
        if (ec) {
            return false;
        }
        boost::asio::write(_client_socket, boost::asio::buffer(bytes), ec);
        _client_socket.non_blocking(true, ec);
        return !ec;
    }

    std::vector<std::unique_ptr<Event>> read_events(std::size_t expected_count, std::chrono::milliseconds timeout) {
        std::vector<std::unique_ptr<Event>> events;
        auto deadline = std::chrono::steady_clock::now() + timeout;
        std::array<uint8_t, 4096> buffer{};

        while (events.size() < expected_count && std::chrono::steady_clock::now() < deadline) {
            boost::system::error_code ec;
            const auto n = _client_socket.read_some(boost::asio::buffer(buffer), ec);
            if (ec == boost::asio::error::would_block || ec == boost::asio::error::try_again) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            if (ec) {
                break;
            }

            auto messages = _client_rx_framer.consume(std::span<const uint8_t>(buffer.data(), n));
            for (const auto& msg : messages) {
                auto event = _client_codec.decodeEvent(msg);
                if (event) {
                    events.push_back(std::move(event));
                }
            }
        }

        return events;
    }

    bool wait_closed(std::chrono::milliseconds timeout) {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        std::array<uint8_t, 64> buffer{};

        while (std::chrono::steady_clock::now() < deadline) {
            boost::system::error_code ec;
            const auto n = _client_socket.read_some(boost::asio::buffer(buffer), ec);
            if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
                return true;
            }
            if (ec == boost::asio::error::would_block || ec == boost::asio::error::try_again) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            if (ec) {
                return true;
            }
            if (n > 0) {
                continue;
            }
        }
        return false;
    }

private:
    void shutdown() {
        boost::system::error_code ec;
        _client_socket.close(ec);

        if (_connection) {
            auto connection = _connection;
            boost::asio::post(_server_io, [connection]() { connection->stop(); });
        }

        _acceptor.close(ec);
        _server_io.stop();
        if (_server_thread.joinable()) {
            _server_thread.join();
        }
    }

    boost::asio::io_context _server_io;
    boost::asio::io_context _client_io;

    Codec _server_codec;
    Codec _client_codec;
    MatchingEngine _matching_engine;

    Connection::pointer _connection;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _client_socket;
    Framer _client_framer;
    Framer _client_rx_framer;

    std::thread _server_thread;
    std::mutex _mx;
    std::condition_variable _cv;
    bool _accept_done = false;
    bool _accept_ok = false;
    bool _ready = false;
    std::string _init_error;
};

void test_limit_roundtrip_ack() {
    TcpV1Harness h;
    if (!h.ready()) {
        std::cout << "[INFO] tcp indisponible: " << h.init_error() << "\n";
        return;
    }

    LimitCommand cmd(7, 101, "ACME", Side::Buy, 100, 5);
    CHECK(h.send_command(cmd));

    auto events = h.read_events(1, std::chrono::milliseconds(800));
    CHECK_EQ(events.size(), 1u);
    if (events.size() != 1) {
        return;
    }
    CHECK(events[0]->getType() == EventType::Ack);
    CHECK_EQ(events[0]->getClientId(), 7u);
    CHECK_EQ(events[0]->getOrderId(), 101u);
}

void test_crossing_limit_roundtrip_fill_and_ack() {
    TcpV1Harness h;
    if (!h.ready()) {
        std::cout << "[INFO] tcp indisponible: " << h.init_error() << "\n";
        return;
    }

    CHECK(h.send_command(LimitCommand(1, 11, "ACME", Side::Sell, 100, 5)));
    auto seed_events = h.read_events(1, std::chrono::milliseconds(800));
    CHECK_EQ(seed_events.size(), 1u);
    if (seed_events.size() != 1 || seed_events[0]->getType() != EventType::Ack) {
        return;
    }

    CHECK(h.send_command(LimitCommand(2, 12, "ACME", Side::Buy, 100, 5)));
    auto events = h.read_events(3, std::chrono::milliseconds(800));
    CHECK_EQ(events.size(), 3u);
    CHECK_EQ(count_type(events, EventType::Ack), 1);
    CHECK_EQ(count_type(events, EventType::Fill), 2);

    const auto* fill_maker = find_fill_for(events, 11);
    const auto* fill_taker = find_fill_for(events, 12);
    CHECK(fill_maker != nullptr);
    CHECK(fill_taker != nullptr);
    if (fill_maker && fill_taker) {
        CHECK_EQ(fill_maker->getQty(), 5);
        CHECK_EQ(fill_taker->getQty(), 5);
        CHECK_EQ(fill_maker->getPrice(), 100);
        CHECK_EQ(fill_taker->getPrice(), 100);
        CHECK_EQ(fill_maker->getMatchId(), fill_taker->getMatchId());
    }
}

void test_fragmented_frame_is_reassembled() {
    TcpV1Harness h;
    if (!h.ready()) {
        std::cout << "[INFO] tcp indisponible: " << h.init_error() << "\n";
        return;
    }

    LimitCommand cmd(8, 201, "ACME", Side::Buy, 101, 2);
    CHECK(h.send_fragmented_command(cmd, 6));

    auto events = h.read_events(1, std::chrono::milliseconds(800));
    CHECK_EQ(events.size(), 1u);
    if (events.size() != 1) {
        return;
    }
    CHECK(events[0]->getType() == EventType::Ack);
    CHECK_EQ(events[0]->getClientId(), 8u);
    CHECK_EQ(events[0]->getOrderId(), 201u);
}

void test_invalid_command_frame_closes_connection() {
    TcpV1Harness h;
    if (!h.ready()) {
        std::cout << "[INFO] tcp indisponible: " << h.init_error() << "\n";
        return;
    }

    const std::vector<uint8_t> invalid_frame = {
        0, 0, 0, 1, 99
    };
    CHECK(h.send_raw(invalid_frame));
    CHECK(h.wait_closed(std::chrono::milliseconds(1200)));
}

} // namespace

int main() {
    run_test("tcp limit => ack", test_limit_roundtrip_ack);
    run_test("tcp limit crossing => fills + ack", test_crossing_limit_roundtrip_fill_and_ack);
    run_test("tcp frame fragmentee => reassembly", test_fragmented_frame_is_reassembled);
    run_test("tcp frame invalide => connexion fermee", test_invalid_command_frame_closes_connection);

    std::cout << "Tests: " << (g_total_tests - g_failed_tests) << "/" << g_total_tests
              << " passed, Checks: " << (g_total_checks - g_failed_checks)
              << "/" << g_total_checks << "\n";

    return g_failed_tests == 0 ? 0 : 1;
}
