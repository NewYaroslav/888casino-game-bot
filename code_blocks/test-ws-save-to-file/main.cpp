#include <iostream>
#include <server_ws.hpp>
#include <nlohmann/json.hpp>
#include <xtime.hpp>
#include <future>
#include <thread>
#include <atomic>
#include <chrono>
#include <limits>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <fstream>

using json = nlohmann::json;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

int main() {
    std::cout << "start!" << std::endl;
    std::ofstream output_file("test.json");
    std::ofstream output_file2("test.txt");
    const uint32_t port = 8080;
    std::shared_ptr<WsServer> server;

    {
        server = std::make_shared<WsServer>();
        server->config.port = port;

        auto &casino = server->endpoint["^/888casino-api/?$"];

        /* принимаем сообщения */
        casino.on_message = [&](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
            auto out_message = in_message->string();
            // std::cout << "888casino Server: Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
            output_file2 << out_message << std::endl;
            try {
                json j = json::parse(in_message->string());
                output_file << std::setw(4) << j << std::endl;

                /* парсим сообщения */
                std::string state;
                std::string type;
                std::string description;
                std::string code;
                if(j["payloadData"]["args"]["state"] != nullptr) {
                    state = j["payloadData"]["args"]["state"];
                }
                if(j["payloadData"]["type"] != nullptr) {
                    type = j["payloadData"]["type"];
                }
                if(j["payloadData"]["args"]["description"] != nullptr) {
                    description = j["payloadData"]["args"]["description"];
                }
                if(j["payloadData"]["args"]["code"] != nullptr) {
                    code = j["payloadData"]["args"]["code"];
                }

                if(type == "roulette.winSpots") {
                    if(description.find("Red") != std::string::npos) {
                        std::cout << "Red" << std::endl;
                    } else
                    if(description.find("Black") != std::string::npos) {
                        std::cout << "Black" << std::endl;
                    } else
                    if(description.find("Green") != std::string::npos || code == "0") {
                        std::cout << "Green" << std::endl;
                    } else {
                        std::cout << "ERROR" << std::endl;
                    }
                }

                if(state == "BETS_OPEN") {
                    std::cout << "BETS_OPEN" << std::endl;
                } else
                if(state == "BETS_CLOSING_SOON") {
                    std::cout << "BETS_CLOSING_SOON" << std::endl;
                } else
                if(state == "BETS_CLOSED_ANNOUNCED") {
                    std::cout << "BETS_CLOSED_ANNOUNCED" << std::endl;
                } else
                if(state == "BETS_CLOSED") {
                    std::cout << "BETS_CLOSED" << std::endl;
                } else
                if(state == "GAME_RESOLVED") {
                    std::cout << "GAME_RESOLVED" << std::endl;
                }
            }
            catch (json::parse_error &e) {
                std::cerr << "WsServer, json parser error: " << std::string(e.what()) << std::endl;
            }
            catch(const json::out_of_range& e) {
                std::cerr << "WsServer, json parser error (json::out_of_range), what: " << std::string(e.what()) << std::endl;
            }
            catch(const json::type_error& e) {
                std::cerr << "WsServer, json parser error (json::type_error), what: " << std::string(e.what()) << std::endl;
            }
            catch (std::exception e) {
                std::cerr << "WsServer, json parser error: " << std::string(e.what()) << std::endl;
            }
            catch(...) {
                std::cerr << "WsServer, json parser error" << std::endl;
            }
        };
        casino.on_open = [&](std::shared_ptr<WsServer::Connection> connection) {
            std::cout << "888casino Server: Opened connection: " << connection.get() << std::endl;
        };

        // See RFC 6455 7.4.1. for status codes
        casino.on_close = [&](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
            std::cout << "888casino Server: Closed connection: " << connection.get() << " with status code: " << status << std::endl;
            output_file.close();
            output_file2.close();
        };
        // Can modify handshake response headers here if needed
        casino.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
            return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
        };

        // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
        casino.on_error = [&](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
            std::cout << "888casino Server: Error in connection " << connection.get() << ". "
            << "Error: " << ec << ", error message: " << ec.message() << std::endl;
        };
    }

    server->start([&](unsigned short port) {
        std::cout << "server run" << std::endl;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::system("pause");
    return 0;
}


