/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 4/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_HTTP_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_HTTP_HPP
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include "Logger.hpp"
#include <lightstreamer/client/transport/providers/HttpProvider.hpp>
#include <lightstreamer/client/protocol/Protocol.hpp>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/client/transport/RequestListener.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>
#include <lightstreamer/client/transport/Transport.hpp>

namespace lightstreamer::client::transport {

        class Http : public Transport {
        protected:
            std::shared_ptr<Logger> log = Logger::getLogger("TRANSPORT_LOG");

            std::shared_ptr<HttpProvider> httpProvider;
            std::shared_ptr<session::SessionThread> sessionThread;

        public:
            Http(std::shared_ptr<session::SessionThread> thread, std::shared_ptr<HttpProvider> httpProvider)
                    : sessionThread(thread), httpProvider(httpProvider) {
                sessionThread->registerShutdownHook(httpProvider->getShutdownHook());
            }

            std::shared_ptr<RequestHandle> sendRequest(std::shared_ptr<protocol::Protocol> protocol, std::shared_ptr<requests::LightstreamerRequest> request, std::shared_ptr<RequestListener> protocolListener, std::unordered_map<std::string, std::string> extraHeaders, Proxy proxy, long tcpConnectTimeout, long tcpReadTimeout) {
                if (!httpProvider) {
                    log->fatal("There is no default HttpProvider, can't connect");
                    return nullptr;
                }

                std::shared_ptr<RequestHandle> connection;
                try {
                    auto httpListener = std::make_shared<MyHttpListener>(protocolListener, request, sessionThread);
                    connection = httpProvider->createConnection(protocol, request, httpListener, extraHeaders, proxy, tcpConnectTimeout, tcpReadTimeout);
                } catch (std::exception& e) {
                    log->error("Error - " + std::string(e.what()));

                    sessionThread->queue([protocolListener]() {
                        protocolListener->onBroken();
                    });
                    return nullptr;
                }
                if (!connection) {
                    // We expect that a closed/broken event will be fired soon
                    return nullptr;
                }

                return std::make_shared<RequestHandleAnonymousInnerClass>(connection);
            }

        private:
            class RequestHandleAnonymousInnerClass : public RequestHandle {
                std::shared_ptr<RequestHandle> connection;

            public:
                RequestHandleAnonymousInnerClass(std::shared_ptr<RequestHandle> conn) : connection(conn) {}

                void close(bool forceConnectionClose) override {
                    connection->close(forceConnectionClose);
                }
            };

            class MyHttpListener : public HttpProvider_HttpRequestListener {
            protected:
                std::shared_ptr<Logger> log = Logger::getLogger("TRANSPORT_LOG");

                std::shared_ptr<RequestListener> listener;
                std::shared_ptr<requests::LightstreamerRequest> request;
                std::shared_ptr<session::SessionThread> sessionThread;

            public:
                MyHttpListener(std::shared_ptr<RequestListener> lst, std::shared_ptr<requests::LightstreamerRequest> req, std::shared_ptr<session::SessionThread> thread)
                        : listener(lst), request(req), sessionThread(thread) {}

                void onMessage(const std::string& message) override {
                    sessionThread->queue([this, message]() {
                        listener->onMessage(message);
                    });
                }

                void onOpen() override {
                    sessionThread->queue([this]() {
                        listener->onOpen();
                    });
                }

                void onClosed() override {
                    sessionThread->queue([this]() {
                        listener->onClosed();
                    });
                }

                void onBroken() override {
                    sessionThread->queue([this]() {
                        listener->onBroken();
                    });
                }
            };
        };

    }

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_HTTP_HPP
