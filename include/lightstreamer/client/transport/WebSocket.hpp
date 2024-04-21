/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 3/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKET_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKET_HPP


#include <memory>
#include <string>
#include <cassert>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <lightstreamer/client/transport/providers/WebSocketProvider.hpp>
#include "lightstreamer/client/session/SessionThread.hpp"
#include <lightstreamer/client/protocol/TextProtocol.hpp>
#include "lightstreamer/client/session/InternalConnectionOptions.hpp"
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/client/protocol/Protocol.hpp>
#include <lightstreamer/client/transport/Transport.hpp>
#include "Logger.hpp"
#include <lightstreamer/client/Proxy.hpp>
#include <lightstreamer/util/threads/ThreadShutdownHook.hpp>

namespace lightstreamer::client::transport {

    static bool disabled;
    enum class InternalState {
        NOT_CONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
        BROKEN,
        UNEXPECTED_ERROR
    };

    class WebSocket : public Transport {
        inline static std::shared_ptr<Logger> log = LogManager::getLogger(Constants::TRANSPORT_LOG);

        std::shared_ptr<session::SessionThread> sessionThread;
        std::shared_ptr<session::InternalConnectionOptions> options;
        std::unique_ptr<providers::WebSocketProvider> wsClient;
        InternalState state = InternalState::NOT_CONNECTED;
        std::string defaultSessionId;

        /**
 * Interface to capture connection opening events.
 */
        class ConnectionListener {
        public:
            /**
             * Called when the connection is successfully established.
             */
            virtual void onOpen() = 0;

            /**
             * Called when the connection cannot be established.
             */
            virtual void onBroken() = 0;
        };

        /**
         * Forwards the messages coming from the data stream to the connection listeners.
         *
         * Note: All the methods must be called by SessionThread to fulfill the contract of WebSocket::open.
         */
        class MySessionRequestListener : public SessionRequestListener {
        private:
            session::SessionThread &sessionThread;
            protocol::TextProtocol::StreamListener &streamListener;
            ConnectionListener &connectionListener;


        public:
            // State must be volatile because it is read by methods not called by Session Thread.
            std::atomic<InternalState> state = InternalState::NOT_CONNECTED;
            MySessionRequestListener(session::SessionThread &sessionThread,
                                     protocol::TextProtocol::StreamListener &streamListener,
                                     ConnectionListener &connListener)
                    : sessionThread(sessionThread), streamListener(streamListener), connectionListener(connListener) {}

            /**
             * Called when the WebSocket connection is opened.
             */
            virtual void onOpen() {
                sessionThread.queue([this]() {
                    if (state == InternalState::DISCONNECTED) {
                        log.warn("onOpen event discarded");
                        return;
                    }
                    state = InternalState::CONNECTED;
                    if (log.isDebugEnabled()) {
                        log.debug("WebSocket transport onOpen: " + std::to_string(state));
                    }
                    connectionListener.onOpen();
                });
            }

            /**
             * Called when a message is received through the WebSocket connection.
             */
            virtual void onMessage(const std::string &frame) {
                sessionThread.queue([this, frame]() {
                    if (state == InternalState::DISCONNECTED) {
                        log.debug("onMessage event discarded: " + frame);
                        return;
                    }
                    streamListener.onMessage(frame);
                });
            }

            /**
             * Called when the WebSocket connection is closed.
             */
            virtual void onClosed() {
                sessionThread.queue([this]() {
                    if (state == InternalState::DISCONNECTED) {
                        log.warn("onClosed event discarded");
                        return;
                    }
                    streamListener.onClosed();
                });
            }

            /**
             * Called when the WebSocket connection is broken.
             */
            virtual void onBroken() {
                sessionThread.queue([this]() {
                    if (state == InternalState::DISCONNECTED) {
                        log.warn("onBroken event discarded");
                        return;
                    }
                    state = InternalState::BROKEN;
                    connectionListener.onBroken();
                    streamListener.onBrokenWS();
                });
            }

            /**
             * Closes this listener and marks the connection as disconnected.
             */
            virtual void close() {
                state = InternalState::DISCONNECTED;
                if (streamListener) {
                    streamListener.disable();
                    streamListener.onClosed();
                }
                if (log.isDebugEnabled()) {
                    log.debug("WebSocket transport (close): " + std::to_string(state));
                }
            }
        };

        /**
         * A dummy WebSocket client used as a placeholder.
         */
        class DummyWebSocketClient : public providers::WebSocketProvider {
        public:
            /**
             * Attempts to establish a WebSocket connection.
             * This is a dummy implementation and does nothing.
             */
            void connect(const std::string &address,
                         std::shared_ptr<SessionRequestListener> networkListener,
                         const std::unordered_map<std::string, std::string> &extraHeaders,
                         const std::string &cookies, std::shared_ptr<Proxy> proxy, long timeout) override {}

            /**
             * Closes the WebSocket connection.
             * This is a dummy implementation and does nothing.
             */
            void disconnect() override {}

            /**
             * Sends a message through the WebSocket connection.
             * This is a dummy implementation and does nothing.
             */
            void send(const std::string &message, std::shared_ptr<RequestListener> listener) override {}

            /**
             * Returns a hook to be called on thread shutdown.
             * This is a dummy implementation and does nothing.
             */
            std::shared_ptr<util::threads::ThreadShutdownHook> getThreadShutdownHook() const override {
                return nullptr;
            }
        };

        MySessionRequestListener sessionListener;

    public:
        WebSocket(std::shared_ptr<session::SessionThread> sessionThread,
                  std::shared_ptr<session::InternalConnectionOptions> options,
                  std::string serverAddress, std::shared_ptr<protocol::TextProtocol::StreamListener> streamListener,
                  std::shared_ptr<ConnectionListener> connListener)
                : sessionThread(sessionThread), options(options) {
            // Simplification for the example: directly create an appropriate WebSocketProvider instance
            wsClient = std::make_unique<transport::providers::WebSocketProvider>();
            open(serverAddress, streamListener, connListener);
            if (log->isDebugEnabled()) {
                log->debug("WebSocket transport - state: " + std::to_string(static_cast<int>(state)));
            }
        }

        /**
         * Opens a WebSocket connection.
         * @param serverAddress Target address.
         * @param streamListener Is exposed to the following connection events: opening, closing, reading a message, catching an error.
         *                       For each event, the corresponding listener method is executed on the SessionThread.
         * @param connListener Is only exposed to the event opening connection. The listener method is executed on the SessionThread.
         */
        void open(std::string serverAddress, std::shared_ptr<protocol::TextProtocol::StreamListener> streamListener,
                  std::shared_ptr<ConnectionListener> connListener) {
            assert(sessionListener.state == InternalState::NOT_CONNECTED);

            sessionThread->registerWebSocketShutdownHook(wsClient->getThreadShutdownHook());
            try {
                auto uri = serverAddress + "lightstreamer";

                auto cookies = providers::CookieHelper::getCookieHeader(uri);
                log->info("Requested cookies for URI " + uri + ": " + cookies);
                wsClient->connect(uri, sessionListener,
                                  options->httpExtraHeadersOnSessionCreationOnly ? nullptr : options->httpExtraHeaders,
                                  cookies, options->proxy, options->retryDelay);
                sessionListener.state = InternalState::CONNECTING;
            } catch (const std::exception &e) {
                // Exception handling logic should be here. In C++, throwing an exception from the catch block is more common.
                log->error("Unexpected error during URI validation. " + std::string(e.what()));
                sessionListener.state = InternalState::UNEXPECTED_ERROR;
                sessionListener.onBroken();
            }
        }

        /**
         * Sends a request over WebSocket.
         *
         * Note: The parameters protocol, extraHeaders, proxy, tcpConnectTimeout, and tcpReadTimeout
         * have no meaning for WebSocket connections and are therefore ignored.
         *
         * @param protocol Ignored in the context of WebSocket.
         * @param request The Lightstreamer request to be sent.
         * @param listener The listener to be notified about the request's completion.
         * @param extraHeaders Ignored in the context of WebSocket.
         * @param proxy Ignored in the context of WebSocket.
         * @param tcpConnectTimeout Ignored in the context of WebSocket.
         * @param tcpReadTimeout Ignored in the context of WebSocket.
         * @return A handle to the request, which can be used to manage the request.
         */
        std::unique_ptr<RequestHandle> sendRequest(
                std::shared_ptr<protocol::Protocol> protocol,
                std::shared_ptr<requests::LightstreamerRequest> request,
                std::shared_ptr<RequestListener> listener,
                const std::map<std::string, std::string> &extraHeaders,
                std::shared_ptr<Proxy> proxy,
                long tcpConnectTimeout,
                long tcpReadTimeout) {
            // Asserting the expected conditions for WebSocket connections.
            assert(extraHeaders.empty() && !proxy && tcpConnectTimeout == 0 && tcpReadTimeout == 0);

            std::string frame =
                    request->getRequestName() + "\r\n" + request->getTransportAwareQueryString(defaultSessionId, false);
            wsClient->send(frame, listener);
            return std::make_unique<RequestHandleAnonymousInnerClass>(*this);
        }

    private:
        /**
         * A RequestHandle implementation that should not be used to close the connection.
         * To close the connection, the WebSocket::close() method should be used instead.
         */
        class RequestHandleAnonymousInnerClass : public RequestHandle {
        private:
            WebSocket &outerInstance;

        public:
            RequestHandleAnonymousInnerClass(WebSocket &outerInstance) : outerInstance(outerInstance) {}

            void close(bool forceConnectionClose) override {
                // Note: This method should not be used. To close the connection, use WebSocket::close() method.
                assert(false);
            }
        };


    public:
        /**
         * Closes the connection.
         */
        void close() {
            // Ensure this is called from the correct thread
            // Debug.Assert(Assertions.SessionThread);

            log.info("Closing wsc ");

            sessionListener.close();
            wsClient.disconnect();
        }

        /**
         * Returns the current state of the WebSocket connection.
         *
         * @return The current state as an InternalState enum.
         */
        InternalState getState() const {
            return sessionListener.state;
        }

        /**
         * Sets the default session ID to be used for requests.
         *
         * @param sessionId The default session ID.
         */
        void setDefaultSessionId(const std::string &sessionId) {
            defaultSessionId = sessionId;
        }


    private:


        /**
         * Checks if the WebSocket functionality is disabled.
         *
         * @return true if WebSocket is disabled, false otherwise.
         */
        static bool isDisabled() {
            return disabled;
        }

        /**
         * Restores WebSocket functionality, enabling it if it was previously disabled.
         */
        static void restore() {
            disabled = false;
        }

        /**
         * Disables the WebSocket functionality.
         */
        static void disable() {
            disabled = true;
        }


    };

    // Definitions of member functions would go here, outside of the class declaration.
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKET_HPP
