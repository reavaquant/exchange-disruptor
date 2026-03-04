#include "server/server.h"
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

int main(int argc, char** argv) {
    if (argc > 1 && std::string_view(argv[1]) == "server") {
        IPVersion ipv = IPVersion::IPv4;
        uint16_t port = 9000;
        std::string symbol = "ACME";

        for (int i = 2; i < argc; ++i) {
            const std::string_view arg(argv[i]);
            if (arg == "--port") {
                if (i + 1 >= argc) {
                    std::cerr << "missing value for --port\n";
                    return 1;
                }
                try {
                    const int parsed = std::stoi(argv[++i]);
                    if (parsed < 1 || parsed > 65535) {
                        std::cerr << "invalid --port value\n";
                        return 1;
                    }
                    port = static_cast<uint16_t>(parsed);
                } catch (...) {
                    std::cerr << "invalid --port value\n";
                    return 1;
                }
                continue;
            }
            if (arg == "--symbol") {
                if (i + 1 >= argc) {
                    std::cerr << "missing value for --symbol\n";
                    return 1;
                }
                symbol = argv[++i];
                continue;
            }
            if (arg == "--ipv6") {
                ipv = IPVersion::IPv6;
                continue;
            }
            if (arg == "--ipv4") {
                ipv = IPVersion::IPv4;
                continue;
            }

            std::cerr << "unknown argument: " << arg << "\n";
            return 1;
        }

        try {
            auto server = Server::create(ipv, port, symbol);
            return server->run();
        } catch (const std::exception& e) {
            std::cerr << "server bootstrap error: " << e.what() << "\n";
            return 1;
        }
    }

    std::cout << __cplusplus << "\n";
    return 0;
}
