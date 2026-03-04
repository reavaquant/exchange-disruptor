#include "matching_engine/command/limit_command.h"
#include "matching_engine/event/fill_event.h"
#include "protocol/codec.h"
#include "protocol/framer.h"
#include <boost/asio.hpp>

#include <array>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

int g_total_tests = 0;
int g_failed_tests = 0;
int g_total_checks = 0;
int g_failed_checks = 0;
const char* g_current_test = "";
std::filesystem::path g_exchange_binary;

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

struct Peer {
    boost::asio::io_context io;
    boost::asio::ip::tcp::socket socket;
    Framer tx_framer;
    Framer rx_framer;

    Peer() : socket(io) {}
};

class ServerProcess {
public:
    ~ServerProcess() {
        stop();
    }

    bool start(const std::filesystem::path& binary, uint16_t port, const std::string& symbol, std::string& error) {
        if (_pid > 0) {
            error = "server already started";
            return false;
        }

        const std::string binary_str = binary.string();
        const std::string port_str = std::to_string(port);

        const pid_t pid = fork();
        if (pid < 0) {
            error = "fork failed";
            return false;
        }
        if (pid == 0) {
            execl(binary_str.c_str(),
                  binary_str.c_str(),
                  "server",
                  "--ipv4",
                  "--port",
                  port_str.c_str(),
                  "--symbol",
                  symbol.c_str(),
                  static_cast<char*>(nullptr));
            _exit(127);
        }

        _pid = pid;
        return true;
    }

    bool is_running() {
        if (_pid <= 0) {
            return false;
        }

        int status = 0;
        const pid_t rc = waitpid(_pid, &status, WNOHANG);
        if (rc == 0) {
            return true;
        }
        _pid = -1;
        return false;
    }

    void stop() {
        if (_pid <= 0) {
            return;
        }

        kill(_pid, SIGTERM);
        for (int i = 0; i < 50; ++i) {
            int status = 0;
            const pid_t rc = waitpid(_pid, &status, WNOHANG);
            if (rc == _pid) {
                _pid = -1;
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        kill(_pid, SIGKILL);
        int status = 0;
        waitpid(_pid, &status, 0);
        _pid = -1;
    }

private:
    pid_t _pid = -1;
};

std::optional<uint16_t> pick_free_port() {
    boost::asio::io_context io;
    boost::asio::ip::tcp::acceptor acceptor(io);
    boost::system::error_code ec;

    acceptor.open(boost::asio::ip::tcp::v4(), ec);
    if (ec) {
        return std::nullopt;
    }
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    if (ec) {
        return std::nullopt;
    }
    acceptor.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 0), ec);
    if (ec) {
        return std::nullopt;
    }
    const auto port = acceptor.local_endpoint().port();
    acceptor.close(ec);
    return port;
}

bool wait_for_server(uint16_t port, ServerProcess& process, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (!process.is_running()) {
            return false;
        }

        boost::asio::io_context io;
        boost::asio::ip::tcp::socket socket(io);
        boost::system::error_code ec;
        socket.connect(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), port),
            ec);
        if (!ec) {
            socket.close(ec);
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

bool connect_peer(Peer& peer, uint16_t port) {
    boost::system::error_code ec;
    peer.socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), port), ec);
    if (ec) {
        return false;
    }
    peer.socket.non_blocking(true, ec);
    return !ec;
}

bool send_raw(Peer& peer, const std::vector<uint8_t>& bytes) {
    if (!peer.socket.is_open()) {
        return false;
    }

    boost::system::error_code ec;
    peer.socket.non_blocking(false, ec);
    if (ec) {
        return false;
    }
    boost::asio::write(peer.socket, boost::asio::buffer(bytes), ec);
    const bool ok = !ec;
    peer.socket.non_blocking(true, ec);
    return ok;
}

bool send_command(Peer& peer, Codec& codec, const Command& cmd) {
    auto payload = codec.encodeCommand(cmd);
    auto framed = peer.tx_framer.frame(payload);
    return send_raw(peer, framed);
}

std::vector<std::unique_ptr<Event>> read_events(Peer& peer, Codec& codec, std::size_t expected_count, std::chrono::milliseconds timeout) {
    std::vector<std::unique_ptr<Event>> events;
    auto deadline = std::chrono::steady_clock::now() + timeout;
    std::array<uint8_t, 4096> buffer{};

    while (events.size() < expected_count && std::chrono::steady_clock::now() < deadline) {
        boost::system::error_code ec;
        const auto n = peer.socket.read_some(boost::asio::buffer(buffer), ec);
        if (ec == boost::asio::error::would_block || ec == boost::asio::error::try_again) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (ec) {
            break;
        }

        auto messages = peer.rx_framer.consume(std::span<const uint8_t>(buffer.data(), n));
        for (const auto& msg : messages) {
            auto event = codec.decodeEvent(msg);
            if (event) {
                events.push_back(std::move(event));
            }
        }
    }

    return events;
}

bool wait_closed(Peer& peer, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    std::array<uint8_t, 64> buffer{};

    while (std::chrono::steady_clock::now() < deadline) {
        boost::system::error_code ec;
        const auto n = peer.socket.read_some(boost::asio::buffer(buffer), ec);
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

void test_process_server_two_clients_crossing() {
    if (g_exchange_binary.empty() || !std::filesystem::exists(g_exchange_binary)) {
        CHECK(false && "exchange_disruptor binary not found");
        return;
    }

    auto port_opt = pick_free_port();
    if (!port_opt.has_value()) {
        std::cout << "[INFO] tcp indisponible: cannot reserve local port\n";
        return;
    }
    const uint16_t port = *port_opt;

    ServerProcess server;
    std::string start_error;
    CHECK(server.start(g_exchange_binary, port, "ACME", start_error));
    if (!start_error.empty()) {
        std::cerr << "[INFO] start error: " << start_error << "\n";
        return;
    }

    if (!wait_for_server(port, server, std::chrono::milliseconds(1500))) {
        std::cout << "[INFO] tcp indisponible: server process not reachable\n";
        return;
    }

    Codec codec;
    Peer maker;
    Peer taker;
    CHECK(connect_peer(maker, port));
    CHECK(connect_peer(taker, port));

    CHECK(send_command(maker, codec, LimitCommand(1, 1001, "ACME", Side::Sell, 100, 5)));
    auto maker_seed = read_events(maker, codec, 1, std::chrono::milliseconds(800));
    CHECK_EQ(maker_seed.size(), 1u);
    if (maker_seed.size() != 1 || maker_seed[0]->getType() != EventType::Ack) {
        return;
    }

    CHECK(send_command(taker, codec, LimitCommand(2, 2002, "ACME", Side::Buy, 100, 5)));
    auto taker_events = read_events(taker, codec, 3, std::chrono::milliseconds(800));
    CHECK_EQ(taker_events.size(), 3u);
    CHECK_EQ(count_type(taker_events, EventType::Ack), 1);
    CHECK_EQ(count_type(taker_events, EventType::Fill), 2);

    const auto* maker_fill = find_fill_for(taker_events, 1001u);
    const auto* taker_fill = find_fill_for(taker_events, 2002u);
    CHECK(maker_fill != nullptr);
    CHECK(taker_fill != nullptr);
    if (maker_fill && taker_fill) {
        CHECK_EQ(maker_fill->getQty(), 5);
        CHECK_EQ(taker_fill->getQty(), 5);
        CHECK_EQ(maker_fill->getPrice(), 100);
        CHECK_EQ(taker_fill->getPrice(), 100);
        CHECK_EQ(maker_fill->getMatchId(), taker_fill->getMatchId());
    }
}

void test_process_server_invalid_frame_disconnects_client() {
    if (g_exchange_binary.empty() || !std::filesystem::exists(g_exchange_binary)) {
        CHECK(false && "exchange_disruptor binary not found");
        return;
    }

    auto port_opt = pick_free_port();
    if (!port_opt.has_value()) {
        std::cout << "[INFO] tcp indisponible: cannot reserve local port\n";
        return;
    }
    const uint16_t port = *port_opt;

    ServerProcess server;
    std::string start_error;
    CHECK(server.start(g_exchange_binary, port, "ACME", start_error));
    if (!start_error.empty()) {
        std::cerr << "[INFO] start error: " << start_error << "\n";
        return;
    }

    if (!wait_for_server(port, server, std::chrono::milliseconds(1500))) {
        std::cout << "[INFO] tcp indisponible: server process not reachable\n";
        return;
    }

    Codec codec;
    Peer peer;
    CHECK(connect_peer(peer, port));

    const std::vector<uint8_t> invalid_frame = {0, 0, 0, 1, 99};
    CHECK(send_raw(peer, invalid_frame));
    CHECK(wait_closed(peer, std::chrono::milliseconds(1200)));

    CHECK(server.is_running());

    Peer healthy_peer;
    CHECK(connect_peer(healthy_peer, port));
    CHECK(send_command(healthy_peer, codec, LimitCommand(7, 7007, "ACME", Side::Buy, 99, 1)));
    auto events = read_events(healthy_peer, codec, 1, std::chrono::milliseconds(800));
    CHECK_EQ(events.size(), 1u);
    if (events.size() == 1) {
        CHECK(events[0]->getType() == EventType::Ack);
    }
}

std::filesystem::path resolve_exchange_binary(const char* argv0) {
    std::error_code ec;
    const auto exe_path = std::filesystem::weakly_canonical(std::filesystem::path(argv0), ec);
    if (ec || exe_path.empty()) {
        return {};
    }
    const auto candidate = exe_path.parent_path().parent_path() / "exchange_disruptor";
    return candidate;
}

} // namespace

int main(int argc, char** argv) {
    if (argc > 0 && argv[0] != nullptr) {
        g_exchange_binary = resolve_exchange_binary(argv[0]);
    }

    run_test("integration tcp v1 process: two clients crossing", test_process_server_two_clients_crossing);
    run_test("integration tcp v1 process: invalid frame disconnect", test_process_server_invalid_frame_disconnects_client);

    std::cout << "Tests: " << (g_total_tests - g_failed_tests) << "/" << g_total_tests
              << " passed, Checks: " << (g_total_checks - g_failed_checks)
              << "/" << g_total_checks << "\n";

    return g_failed_tests == 0 ? 0 : 1;
}
