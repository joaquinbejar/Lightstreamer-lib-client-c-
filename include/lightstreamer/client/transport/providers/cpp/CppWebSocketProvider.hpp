/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 25/4/24
 ******************************************************************************/

/*******************************************************************************
 Copyright (c) 2024.

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program. If not, see <https://www.gnu.org/licenses/>..
 ******************************************************************************/

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CPPWEBSOCKETPROVIDER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CPPWEBSOCKETPROVIDER_HPP
#include <memory>
#include <regex>
#include <string>
#include <exception>
#include <unordered_map>


#include <lightstreamer/client/transport/providers/WebSocketProvider.hpp>
#include "SessionRequestListener.h"
#include "RequestListener.h"
#include "WebSocketPoolManager.h"
#include "LsUtils.h"
#include "PipelineUtils.h"
#include "SingletonFactory.h"

namespace lightstreamer::client::transport::providers::cpp {

    /**
     * WebSocket client based on Cpp.
     * The implementation is modeled after this example: https://github.com/cpp/cpp/tree/4.1/example/src/main/java/io/cpp/example/http/websocketx/client.
     * This class notifies a SessionRequestListener when the following events happen:
     * - onOpen: fires when the connection is established and the WebSocket handshake is complete.
     * - onMessage: fires when a new text frame is received.
     * - onClosed: fires when the connection is closed.
     * - onBroken: fires when there is an error.
     * 
     * NB1: The current implementation allows the sending of cookies in the handshake request but doesn't
     * support the setting of cookies in the handshake response.
     * 
     * NB2: The actual implementation limits to 64Kb the maximum frame size.
     * This is not a problem because the Lightstreamer server sends frames whose size is at most 8Kb.
     * The limit can be modified specifying a different size at the creation of the WebSocketClientHandshaker.
     */
    class CppWebSocketProvider : public WebSocketProvider {
        inline static std::shared_ptr<ILogger> log = LogManager::GetLogger(Constants::NETTY_LOG);
        inline static std::shared_ptr<ILogger> logStream = LogManager::GetLogger(Constants::TRANSPORT_LOG);
        inline static std::shared_ptr<ILogger> logPool = LogManager::GetLogger(Constants::NETTY_POOL_LOG);

        std::shared_ptr<WebSocketPoolManager> wsPoolManager;
        std::shared_ptr<MyChannel> channel;

    public:
        CppWebSocketProvider() : wsPoolManager(SingletonFactory::instance().WsPool) {}

        // TEST ONLY
        explicit CppWebSocketProvider(std::shared_ptr<WebSocketPoolManager> channelPool) : wsPoolManager(channelPool) {}

        void connect(const std::string& address, std::shared_ptr<SessionRequestListener> networkListener,
                     const std::unordered_map<std::string, std::string>& extraHeaders, const std::string& cookies,
                     Proxy proxy, long timeout) {
            auto uri = LsUtils::uri(address);
            std::string host = uri.host;
            int port = LsUtils::port(uri);
            bool secure = LsUtils::isSSL(uri);

            try {
                std::string host4Cpp = System::Net::Dns::GetHostAddresses(host)[0].ToString();

                CppFullAddress remoteAddress(secure, host4Cpp, port, host, proxy);
                ExtendedCppFullAddress extendedRemoteAddress(remoteAddress, extraHeaders, cookies);

                auto wsPool = std::dynamic_pointer_cast<WebSocketChannelPool>(wsPoolManager->get(extendedRemoteAddress));

                auto ch = wsPool->AcquireNewOr(timeout);

                if (ch) {
                    if (ch->Active()) {
                        channel = std::make_shared<MyChannel>(ch, wsPool, networkListener);
                        auto chHandler = std::make_shared<WebSocketChannelHandler>(networkListener, channel);
                        PipelineUtils::populateWSPipeline(ch, chHandler);
                        networkListener->onOpen();
                    } else {
                        log->Error("WebSocket handshake error, ");
                        networkListener->onBroken();
                    }
                } else {
                    log->Error("WebSocket handshake error, channel unexpectedly null");
                    networkListener->onBroken();
                }
            } catch (std::exception& e) {
                log->Error("WebSocket handshake error: " + std::string(e.what()));
                networkListener->onBroken();
            }
        }

        void send(const std::string& message, std::shared_ptr<RequestListener> listener) {
            if (logStream->IsDebugEnabled()) {
                logStream->Debug("WS transport sending [" + std::to_string(channel) + "]: " + message);
            }
            channel->write(message, listener);
        }

        void disconnect() {
            if (channel) {
                if (logPool->IsDebugEnabled()) {
                    logPool->Debug("WS disconnect [" + std::to_string(channel) + "]");
                }

                channel->close();
                channel.reset();
            }
        }

        std::shared_ptr<ThreadShutdownHook> getThreadShutdownHook() {
            return nullptr; // nothing to do
        }

    private:
        // Additional private class definitions (MyChannel, WebSocketChannelHandler, RequestListenerDecorator) go here.
    };

} // namespace lightstreamer::client::transport::providers::cpp

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CPPWEBSOCKETPROVIDER_HPP
