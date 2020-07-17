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


using json = nlohmann::json;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

int main() {
    std::cout << "start!" << std::endl;

    const uint32_t port = 8080;
    std::shared_ptr<WsServer> server;

    {
        server = std::make_shared<WsServer>();
        server->config.port = port;

        auto &olymptrade = server->endpoint["^/888casino-api/?$"];

        /* принимаем сообщения */
        olymptrade.on_message = [&](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
            auto out_message = in_message->string();
            std::cout << "888casino Server: Message received: \"" << out_message << "\" from " << connection.get() << std::endl;
            try {
                //json j = json::parse(in_message->string());
            }
            catch(...) {}
        };
        olymptrade.on_open = [&](std::shared_ptr<WsServer::Connection> connection) {
            std::cout << "888casino Server: Opened connection: " << connection.get() << std::endl;
        };

        // See RFC 6455 7.4.1. for status codes
        olymptrade.on_close = [&](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
            std::cout << "888casino Server: Closed connection: " << connection.get() << " with status code: " << status << std::endl;
        };
        // Can modify handshake response headers here if needed
        olymptrade.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
            return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
        };

        // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
        olymptrade.on_error = [&](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
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


