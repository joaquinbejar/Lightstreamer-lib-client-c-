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
#include <lightstreamer/client/transport/SessionRequestListener.hpp>
#include <lightstreamer/client/transport/RequestListener.hpp>
#include <lightstreamer/client/transport/providers/cpp/pool/WebSocketPoolManager.hpp>
#include <lightstreamer/util/LsUtils.hpp>
#include <lightstreamer/client/transport/providers/cpp/PipelineUtils.hpp>
#include <lightstreamer/client/transport/providers/cpp/SingletonFactory.hpp>
#include <lightstreamer/util/threads/ThreadShutdownHook.hpp>
#include <lightstreamer/client/protocol/TextProtocol.hpp>

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

        /**
 * Netty channel wrapper.
 * This class is synchronized because its methods are called from both session thread and Netty thread.
 */
        class MyChannel {
            std::shared_ptr<IChannel> ch;
            std::shared_ptr<IChannelPool> pool;
            std::shared_ptr<SessionRequestListener> networkListener;
            bool closed = false;
            bool released = false;

        public:
            MyChannel(std::shared_ptr<IChannel> ch, std::shared_ptr<IChannelPool> pool, std::shared_ptr<SessionRequestListener> networkListener)
                    : ch(ch), pool(pool), networkListener(networkListener) {}

            void write(const std::string& message, std::shared_ptr<RequestListener> listener) {
                std::lock_guard<std::mutex> lock(mutex); // Replaces `lock (this)`
                if (closed || released) {
                    log->Debug("Message discarded because the channel [" + std::to_string(ch->Id()) + "] is closed: " + message);
                    return;
                }
                if (listener) {
                    // Moved the onOpen call outside of operationComplete write callback
                    listener->onOpen();
                }
                auto chf = ch->WriteAndFlushAsync(message);
                chf->ContinueWith([this, message](std::future<void> future) {
                    try {
                        future.get(); // Waits for the task to complete and rethrows exception if any
                    } catch (const std::exception& e) {
                        this->onBroken(message, e);
                    }
                });
            }

            /**
             * Releases the channel to its pool.
             */
            void release() {
                std::lock_guard<std::mutex> lock(mutex);
                log->Debug("Release [" + std::to_string(ch->Id()) + "]");

                if (!closed && !released) {
                    released = true;
                    pool->ReleaseAsync(ch);
                }
            }

            /**
             * Closes the channel if it has not been released yet.
             */
            void close() {
                std::lock_guard<std::mutex> lock(mutex);
                if (!closed) {
                    logPool->Debug("WS channel closed [" + std::to_string(ch->Id()) + "]");
                    try {
                        ch->CloseAsync().get(); // Asynchronously close and wait for it to complete
                    } catch (const std::exception& e) {
                        logPool->Debug("Something wrong waiting channel close, skip. [" + std::to_string(ch->Id()) + "]");
                    }
                    closed = true;
                } else {
                    logPool->Info("WS channel already closed [" + std::to_string(ch->Id()) + "]");
                }

                if (!released) {
                    pool->ReleaseAsync(ch);
                } else {
                    logPool->Info("WS channel already released [" + std::to_string(ch->Id()) + "]");
                }
            }

            void onBroken(const std::string& message, const std::exception& cause) {
                std::lock_guard<std::mutex> lock(mutex);
                log->Error("Websocket write failed [" + std::to_string(ch->Id()) + "]: " + message + ", " + cause.what());
                close();
                networkListener->onBroken();
            }

            std::string toString() const {
                std::lock_guard<std::mutex> lock(mutex);
                return std::to_string(ch->Id());
            }

            bool isClosed() const {
                return closed;
            }
        };

        std::shared_ptr<MyChannel> channel;

        /**
         * Parses the messages coming from a channel and forwards them to the corresponding RequestListener.
         */
        class WebSocketChannelHandler : public SimpleChannelInboundHandler<IByteBuffer> {
            std::unique_ptr<LineAssembler> lineAssembler;
            std::shared_ptr<RequestListenerDecorator> reqListenerDecorator;

        public:
            WebSocketChannelHandler(std::shared_ptr<RequestListener> networkListener, std::shared_ptr<MyChannel> ch) {
                reqListenerDecorator = std::make_shared<RequestListenerDecorator>(networkListener, ch);
                lineAssembler = std::make_unique<LineAssembler>(reqListenerDecorator);
            }

            void channelRead0(std::shared_ptr<IChannelHandlerContext> ctx, std::shared_ptr<IByteBuffer> msg) override {
                lineAssembler->readBytes(msg);
            }

            void handlerAdded(std::shared_ptr<IChannelHandlerContext> ctx) {
                if (log->IsDebugEnabled()) {
                    auto ch = ctx->channel();
                    log->Debug("Handler added [" + std::to_string(ch->Id()) + "]");
                }
                reqListenerDecorator->onOpen();
            }

            void channelActive(std::shared_ptr<IChannelHandlerContext> ctx) {
                if (log->IsDebugEnabled()) {
                    auto ch = ctx->channel();
                    log->Debug("WebSocket active [" + std::to_string(ch->Id()) + "]");
                }
                ctx->fireChannelActive();
            }

            void channelInactive(std::shared_ptr<IChannelHandlerContext> ctx) {
                if (log->IsDebugEnabled()) {
                    log->Debug("WebSocket disconnected [" + std::to_string(ctx->channel()->Id()) + "]");
                }
                reqListenerDecorator->onClosed();
            }

            void exceptionCaught(std::shared_ptr<IChannelHandlerContext> ctx, const std::exception& cause) {
                if (log->IsDebugEnabled()) {
                    log->Error("WebSocket error [" + std::to_string(ctx->channel()->Id()) + "]: " + cause.what());
                }
                reqListenerDecorator->onBroken();
            }
        };

        /**
         * A RequestListener which releases the connection to its pool when the method onMessage encounters
         * the message "LOOP" or "END".
         */
        class RequestListenerDecorator : public RequestListener {
            std::shared_ptr<RequestListener> listener;
            std::shared_ptr<MyChannel> ch;

        public:
            RequestListenerDecorator(std::shared_ptr<RequestListener> listener, std::shared_ptr<MyChannel> ch)
                    : listener(listener), ch(ch) {}

            void onMessage(const std::string& message) override {
                if (ch->isClosed()) {
                    log->Info("Message received despite the channel being closed: " + message);
                }
                listener->onMessage(message);

                std::regex loop_regex(protocol::TextProtocol::LOOP_REGEX);
                std::regex end_regex(protocol::TextProtocol::END_REGEX);
                if (std::regex_search(message, loop_regex)) {
                    ch->release();
                } else if (std::regex_search(message, end_regex)) {
                    ch->close();
                }
            }

            void onOpen() override {
                listener->onOpen();
            }

            void onClosed() override {
                listener->onClosed();
                log->Debug("OnClosed event fired for channel: " + std::to_string(ch->ch->Id()));
            }

            void onBroken() override {
                listener->onBroken();
            }
        };



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

        std::shared_ptr<util::threads::ThreadShutdownHook> getThreadShutdownHook() {
            return nullptr; // nothing to do
        }


    };

} // namespace lightstreamer::client::transport::providers::cpp

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CPPWEBSOCKETPROVIDER_HPP
