#ifndef CASIO_WS_SERVER_HPP_INCLUDED
#define CASIO_WS_SERVER_HPP_INCLUDED

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

namespace casino {

    class Server {
    private:
        using json = nlohmann::json;
        using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
        const uint32_t port = 8080;
        std::shared_ptr<WsServer> server;
        std::future<void> server_future;
        std::atomic<bool> is_shutdown;

    public:

        /// Типы событий
        enum class EventTypes {
            BETS_OPEN,
            BETS_CLOSING_SOON,
            BETS_CLOSED_ANNOUNCED,
            BETS_CLOSED,
            GAME_RESOLVED,
            CONNECT,
            DISCONNECT,
            GAME_CONNECT,
            GAME_DISCONNECT,
            WIN_SPOTS,
            SERVER_RUN,
            ERROR_MESSAGE,
        };

        enum class WinBetTypes {
            NONE,
            RED,
            BLACK,
            ZERO,
            ERROR_MESSAGE,
        };

        std::function<void(EventTypes event, WinBetTypes bet_type, const std::string &msg)> on_event;

        Server() {};

        void init() {
            is_shutdown = false;
            /* запускаем поток сервера */
            server_future = std::async(std::launch::async,[&]() {
                while(!is_shutdown) {
                    server = std::make_shared<WsServer>();
                    server->config.port = port;
                    auto &casino = server->endpoint["^/888casino-api/?$"];

                    /* принимаем сообщения */
                    casino.on_message = [&](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
                        auto out_message = in_message->string();
                        std::cout << "888casino Server: Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
                        try {
                            json j = json::parse(in_message->string());

                            if(j["event"] != nullptr) {
                                if(j["event"] == "disconnect") {
                                    if(on_event != nullptr) on_event(EventTypes::GAME_DISCONNECT, WinBetTypes::NONE, "GAME_DISCONNECT");
                                } else
                                if(j["event"] == "connect") {
                                    if(on_event != nullptr) on_event(EventTypes::GAME_CONNECT, WinBetTypes::NONE, "GAME_CONNECT");
                                } else
                                if(j["event"] == "start") {
                                    std::cout << "TEST START!" << std::endl;
                                }
                            }

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
                                    if(on_event != nullptr) on_event(EventTypes::WIN_SPOTS, WinBetTypes::RED, "RED");
                                    std::cout << "Red" << std::endl;
                                    //casino::Strategy::Bet bet;
                                    //strategy.update(casino::Strategy::TypesSignal::RED, bet);
                                    //if(bet.step == 0) std::cout << "skip" << std::endl << std::endl;
                                    //else std::cout << "step " << bet.step  << " color " << bet.get_color() << std::endl << std::endl;
                                } else
                                if(description.find("Black") != std::string::npos) {
                                    std::cout << "Black" << std::endl;
                                    if(on_event != nullptr) on_event(EventTypes::WIN_SPOTS, WinBetTypes::BLACK, "BLACK");
                                    //casino::Strategy::Bet bet;
                                    //strategy.update(casino::Strategy::TypesSignal::BLACK, bet);
                                    //if(bet.step == 0) std::cout << "skip" << std::endl << std::endl;
                                    //else std::cout << "step " << bet.step  << " color " << bet.get_color() << std::endl << std::endl;
                                } else
                                if(description.find("Green") != std::string::npos || code == "0") {
                                    std::cout << "Zero" << std::endl;
                                    if(on_event != nullptr) on_event(EventTypes::WIN_SPOTS, WinBetTypes::ZERO, "ZERO");
                                    //casino::Strategy::Bet bet;
                                    //strategy.update(casino::Strategy::TypesSignal::ZERO, bet);
                                    //if(bet.step == 0) std::cout << "skip" << std::endl << std::endl;
                                } else {
                                    std::cout << "ERROR" << std::endl;
                                    if(on_event != nullptr) on_event(EventTypes::WIN_SPOTS, WinBetTypes::ERROR_MESSAGE, "ERROR");
                                }
                            }

                            if(state == "BETS_OPEN") {
                                if(on_event != nullptr) on_event(EventTypes::BETS_OPEN, WinBetTypes::NONE, "BETS_OPEN");
                            } else
                            if(state == "BETS_CLOSING_SOON") {
                                if(on_event != nullptr) on_event(EventTypes::BETS_CLOSING_SOON, WinBetTypes::NONE, "BETS_CLOSING_SOON");
                            } else
                            if(state == "BETS_CLOSED_ANNOUNCED") {
                                if(on_event != nullptr) on_event(EventTypes::BETS_CLOSED_ANNOUNCED, WinBetTypes::NONE, "BETS_CLOSED_ANNOUNCED");
                            } else
                            if(state == "BETS_CLOSED") {
                                if(on_event != nullptr) on_event(EventTypes::BETS_CLOSED, WinBetTypes::NONE, "BETS_CLOSED");
                            } else
                            if(state == "GAME_RESOLVED") {
                                if(on_event != nullptr) on_event(EventTypes::GAME_RESOLVED, WinBetTypes::NONE, "GAME_RESOLVED");
                            }
                        }
                        catch (json::parse_error &e) {
                            std::cerr << "888casino Server, json parser error: " << std::string(e.what()) << std::endl;
                            std::cout << "Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
                            if(on_event != nullptr) on_event(EventTypes::ERROR_MESSAGE, WinBetTypes::NONE, std::string("json parser error: " + std::string(e.what())));
                        }
                        catch(const json::out_of_range& e) {
                            std::cerr << "888casino Server, json parser error (json::out_of_range), what: " << std::string(e.what()) << std::endl;
                            std::cout << "Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
                            if(on_event != nullptr) on_event(EventTypes::ERROR_MESSAGE, WinBetTypes::NONE, std::string("json parser error: " + std::string(e.what())));
                        }
                        catch(const json::type_error& e) {
                            std::cerr << "888casino Server, json parser error (json::type_error), what: " << std::string(e.what()) << std::endl;
                            std::cout << "Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
                            if(on_event != nullptr) on_event(EventTypes::ERROR_MESSAGE, WinBetTypes::NONE, std::string("json parser error: " + std::string(e.what())));
                        }
                        catch (std::exception e) {
                            std::cerr << "888casino Server, json parser error: " << std::string(e.what()) << std::endl;
                            std::cout << "Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
                            if(on_event != nullptr) on_event(EventTypes::ERROR_MESSAGE, WinBetTypes::NONE, std::string("json parser error: " + std::string(e.what())));
                        }
                        catch(...) {
                            std::cerr << "888casino Server, json parser error" << std::endl;
                            std::cout << "Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
                            if(on_event != nullptr) on_event(EventTypes::ERROR_MESSAGE, WinBetTypes::NONE, std::string("json parser error: undefined"));
                        }
                    };
                    casino.on_open = [&](std::shared_ptr<WsServer::Connection> connection) {
                        std::cout << "888casino Server: Opened connection: " << connection.get() << std::endl;
                        if(on_event != nullptr) on_event(EventTypes::CONNECT, WinBetTypes::NONE, "888casino Server: Opened connection");
                    };

                    // See RFC 6455 7.4.1. for status codes
                    casino.on_close = [&](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
                        std::cout << "888casino Server: Closed connection: " << connection.get() << " with status code: " << status << std::endl;
                        if(on_event != nullptr) on_event(EventTypes::DISCONNECT, WinBetTypes::NONE, "888casino Server: Closed connection");
                    };
                    // Can modify handshake response headers here if needed
                    casino.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
                        return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
                    };

                    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                    casino.on_error = [&](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
                        std::cout << "888casino Server: Error in connection " << connection.get() << ". "
                        << "Error: " << ec << ", error message: " << ec.message() << std::endl;
                        if(on_event != nullptr) on_event(EventTypes::ERROR_MESSAGE, WinBetTypes::NONE, "888casino Server: Error in connection");
                    };

                    server->start([&](unsigned short port) {
                        std::cout << "888casino Server Run" << std::endl;
                        if(on_event != nullptr) on_event(EventTypes::SERVER_RUN, WinBetTypes::NONE, "server run, port: " + std::to_string(port));
                    });
                }
            });
        }

        ~Server() {
            is_shutdown = true;
            if(server) {
                server->stop();
            }
            if(server_future.valid()) {
                try {
                    server_future.wait();
                    server_future.get();
                }
                catch(const std::exception &e) {
                    std::cerr << "Error: ~WsServer(), server_future, what: " << e.what() << std::endl;
                }
                catch(...) {
                    std::cerr << "Error: ~WsServer(), server_future" << std::endl;
                }
            }
        }

    };
}

#endif // CASIO_WS_SERVER_HPP_INCLUDED
