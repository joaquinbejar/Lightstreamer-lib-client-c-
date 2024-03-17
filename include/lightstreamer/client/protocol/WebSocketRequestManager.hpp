/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETREQUESTMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETREQUESTMANAGER_HPP

#include <list>
#include <memory>
#include <unordered_map>
#include "WebSocket.h" // Assume WebSocket is a class defined elsewhere.
#include <lightstreamer/client/transport/RequestListener.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>
#include "InternalConnectionOptions.h"
#include <lightstreamer/client/protocol/RequestManager.hpp>

namespace lightstreamer::client::protocol {

    /**
     * The manager forwards the requests to the WebSocket transport ensuring that if the underlying connection is not ready,
     * the requests are buffered and sent later.
     *
     * Note 1:
     * The method `openSocket(std::string, StreamListener*)` is used when the flag `isEarlyWSOpenEnabled` is set. If the method is not called explicitly,
     * the method `bindSession(SessionRequest*, transport::RequestListener*, long, long, std::future<void>*)` will call it.
     *
     * Note 2:
     * If the method `openSocket(std::string, StreamListener*)` is called twice in a row (this can happen if the server sends a control-link),
     * the final effect is to close the old socket and to open a new one.
     */
    class WebSocketRequestManager : public RequestManager {
        ILogger &log;
        ILogger &sessionLog;
        std::unique_ptr<WebSocket> wsTransport;
        Protocol &protocol;
        session::SessionThread &sessionThread;
        InternalConnectionOptions &options;
        std::list<PendingRequest> controlRequestQueue;
        std::unique_ptr<PendingBind> bindRequest;

        /**
         * @brief A request that the manager has sent but has not yet been written to the WebSocket.
         * This must be cleared when transport::RequestListener::onOpen() is called, assuming that the WebSocket is reliable.
         */
        PendingRequest ongoingRequest;
        /**
         * @brief Maps the LS_reqId of a request to the request's listener.
         */
        std::unordered_map<long, std::shared_ptr<transport::RequestListener>> pendingRequestMap;
        ListenableFuture openWsFuture;

        class MyRunnableError {
            std::shared_ptr<StreamListener> reqListener;
            std::shared_ptr<ILogger> log;

        public:
            MyRunnableError(std::shared_ptr<StreamListener> listener, std::shared_ptr<ILogger> logger)
                    : reqListener(listener), log(logger) {}

            // Constructor por defecto
            MyRunnableError() = default;

            // Función que se invoca para ejecutar la tarea
            void operator()() {
                log->Warn("WebSocketRequestManager - MyRunnableError run");
                reqListener->onBrokenWS();
            }
        };

        class RequestHandleConcrete : public RequestHandle {
            WebSocketRequestManager &outerInstance;

        public:
            RequestHandleConcrete(WebSocketRequestManager &outerInstance) : outerInstance(outerInstance) {}

            void close(bool forceConnectionClose) override {
                outerInstance.close(false);
            }
        };

        void
        sendControlRequest(const LightstreamerRequest &request, std::shared_ptr<transport::RequestListener> reqListener,
                           const RequestTutor &tutor) {
            ongoingRequest = PendingRequest(request, reqListener, tutor);
            wsTransport->sendRequest(*protocol, request,
                                     std::make_shared<ListenerWrapperAnonymousInnerClass>(*this, reqListener), nullptr,
                                     nullptr, 0, 0);
        }

        class ListenerWrapperAnonymousInnerClass : public ListenerWrapper {
        private:
            WebSocketRequestManager *outerInstance;

        public:
            // Constructor
            ListenerWrapperAnonymousInnerClass(WebSocketRequestManager *outerInstance, RequestListener *reqListener)
                    : ListenerWrapper(outerInstance, reqListener), outerInstance(outerInstance) {}

            // Implementación de doOpen
            void doOpen() override {
                // La solicitud se ha enviado: limpiar el campo
                outerInstance->ongoingRequest = nullptr;
            }
        };

        void sendBindRequest(LightstreamerRequest *request, transport::RequestListener *reqListener,
                             ListenableFuture *bindFuture) {
            wsTransport.sendRequest(protocol, request, new ListenerWrapper(this, reqListener), nullptr, nullptr, 0, 0);
            bindFuture->fulfill();
        }

        class MyConnectionListener : public WebSocket::ConnectionListener {
        private:
            WebSocketRequestManager *outerInstance;

        public:
            // Constructor
            MyConnectionListener(WebSocketRequestManager *outerInstance)
                    : outerInstance(outerInstance) {}

            // Implementation of onOpen
            void onOpen() override {
                outerInstance->openWsFuture.fulfill();
                // Send bind_session
                if (outerInstance->bindRequest != nullptr) {
                    // Bind request takes precedence over control requests
                    outerInstance->sendBindRequest(outerInstance->bindRequest->request,
                                                   outerInstance->bindRequest->reqListener,
                                                   outerInstance->bindRequest->bindFuture);
                }
                // Send control requests
                for (const auto &controlRequest: outerInstance->controlRequestQueue) {
                    outerInstance->sendControlRequest(controlRequest.request, controlRequest.reqListener,
                                                      controlRequest.tutor);
                }
                // Release memory
                outerInstance->bindRequest = nullptr;
                outerInstance->controlRequestQueue.clear();
            }

            // Implementation of onBroken
            void onBroken() override {
                // NB: The callback caller must ensure execution on SessionThread
                outerInstance->openWsFuture.reject();
            }
        };

        class PendingRequest {
        public:
            // Members
            std::shared_ptr<LightstreamerRequest> request;
            std::shared_ptr<transport::RequestListener> reqListener;
            std::shared_ptr<RequestTutor> tutor;

            // Constructor
            PendingRequest(std::shared_ptr<LightstreamerRequest> request,
                           std::shared_ptr<transport::RequestListener> reqListener, std::shared_ptr<RequestTutor> tutor)
                    : request(request), reqListener(reqListener), tutor(tutor) {}
        };

        class PendingBind : public PendingRequest {
        public:
            // Member
            std::shared_ptr<ListenableFuture> bindFuture;

            // Constructor
            PendingBind(std::shared_ptr<LightstreamerRequest> request,
                        std::shared_ptr<transport::RequestListener> reqListener,
                        std::shared_ptr<ListenableFuture> bindFuture)
                    : PendingRequest(request, reqListener, nullptr), bindFuture(bindFuture) {}
        };

        /// \brief A wrapper ensuring that the method `RequestListener::onOpen()` is executed
        ///        in the SessionThread.
        class ListenerWrapper : public transport::RequestListener {
        private:
            WebSocketRequestManager *outerInstance;
            RequestListener *reqListener;

        public:
            ListenerWrapper(WebSocketRequestManager *outerInstance, RequestListener *listener)
                    : outerInstance(outerInstance), reqListener(listener) {
            }

            /// \brief Extra operations to perform before executing `RequestListener::onOpen()`.
            void doOpen() {
                // Add any necessary logic here.
            }

            void onOpen() override {
                outerInstance->sessionThread.queue([this]() {
                    doOpen();
                    reqListener->onOpen();
                });
            }


            void onMessage(const std::string &message) override {
                reqListener->onMessage(message);
            }

            void onClosed() override {
                reqListener->onClosed();
            }

            void onBroken() override {
                reqListener->onBroken();
            }
        };


    public:
        WebSocketRequestManager(std::shared_ptr<session::SessionThread> thread, std::shared_ptr<Protocol> prot,
                                std::shared_ptr<InternalConnectionOptions> opts)
                : options(opts), sessionThread(thread), protocol(prot) {}

        std::future<void> openWS(std::shared_ptr<Protocol> prot, const std::string &serverAddress,
                                 std::shared_ptr<StreamListener> streamListener) {
            if (wsTransport) {
                // Close old connection
                wsTransport->close();
            }

            wsTransport = std::make_unique<WebSocket>(sessionThread, options, serverAddress, streamListener,
                                                      std::make_unique<MyConnectionListener>(*this));

            assert(wsTransport->getState() == WebSocket::InternalState::CONNECTING);

            auto future = openWsPromise.get_future();
            // Abort connection if opening takes too long
            auto wsTransportCopy = wsTransport.get();
            auto timeout = options->getCurrentConnectTimeout();

            sessionThread->schedule([wsTransportCopy, timeout, this]() {
                if (wsTransportCopy->getState() == WebSocket::InternalState::CONNECTING ||
                    wsTransportCopy->getState() == WebSocket::InternalState::UNEXPECTED_ERROR) {
                    this->log->Debug("WS connection: aborted");
                    openWsPromise.set_value();
                    wsTransportCopy->close();
                    options->increaseConnectTimeout();
                }
            }, timeout);

            return future;
        }

        /**
         * Attempts to bind a session using a WebSocket connection. If the WebSocket is not already open,
         * it initiates an attempt to open a WebSocket connection.
         *
         * @param request The bind session request details.
         * @param reqListener Listener to handle stream-related events.
         * @param tcpConnectTimeout Timeout in milliseconds for the TCP connection.
         * @param tcpReadTimeout Timeout in milliseconds for reading from the TCP connection.
         * @param bindFuture A future object to signal the completion of the bind operation.
         * @return A handle to the request, which can be used to close the stream connection.
         */
        std::shared_ptr<RequestHandle>
        bindSession(std::shared_ptr<BindSessionRequest> request, std::shared_ptr<StreamListener> reqListener,
                    long tcpConnectTimeout, long tcpReadTimeout, std::promise<void> &bindFuture) {
            if (!wsTransport) {
                // No transport: this can occur when transport is in polling mode
                bindRequest = std::make_unique<PendingBind>(request, reqListener, bindFuture);
                log->info("WebSocket Manager .. bindSession p2: " +
                          std::to_string(reinterpret_cast<std::uintptr_t>(wsTransport.get())));
                auto future = openWS(*protocol, request->getTargetServer(), reqListener);
                future.onRejected([reqListener, this]() {
                    MyRunnableError errorAction(reqListener, log);
                    errorAction();
                });
            } else {
                // Transport exists, meaning openSocket has already been called; the state is CONNECTED or CONNECTING
                auto state = wsTransport->getState();
                switch (state) {
                    case WebSocket::InternalState::CONNECTED:
                        sendBindRequest(*request, *reqListener, bindFuture);
                        break;

                    case WebSocket::InternalState::CONNECTING:
                        // Buffer the request, to be flushed once the connection state is CONNECTED
                        assert(!bindRequest); // Ensure there's no existing bind request
                        bindRequest = std::make_unique<PendingBind>(request, reqListener, bindFuture);
                        break;

                    case WebSocket::InternalState::BROKEN:
                        // Discard bind request; it must be sent via HTTP
                        break;

                    default:
                        // Unexpected state for a bind request
                        sessionLog->warn("Unexpected bind request in state " + std::to_string(static_cast<int>(state)));
                        break;
                }
            }
            // The returned request handle can close the stream connection
            return std::make_shared<RequestHandleAnonymousInnerClass>(*this);
        }

        void addRequest(const LightstreamerRequest &request, const RequestTutor &tutor,
                        std::shared_ptr<transport::RequestListener> reqListener) {
            assert(dynamic_cast<const ControlRequest *>(&request) || dynamic_cast<const MessageRequest *>(&request) ||
                   dynamic_cast<const ReverseHeartbeatRequest *>(&request));
            if (const auto numberedReq = dynamic_cast<const NumberedRequest *>(&request)) {
                // Para solicitudes numeradas (es decir, con un LS_reqId), el cliente espera una notificación REQOK/REQERR del servidor.
                assert(pendingRequestMap.find(numberedReq->getRequestId()) == pendingRequestMap.end());
                pendingRequestMap[numberedReq->getRequestId()] = reqListener;
                sessionLog.debug("Pending request - post - " + std::to_string(numberedReq->getRequestId()));
            }
            if (!wsTransport) {
                // Sin transporte: este caso puede ocurrir, por ejemplo, cuando la bandera isEarlyWSOpenEnabled está desactivada.
                // Se almacena la solicitud en búfer y se espera la vinculación de la sesión.
                controlRequestQueue.emplace_back(request, reqListener, tutor);
            } else {
                // Hay transporte, por lo que ya se llamó a openSocket: el estado es CONNECTED o CONNECTING.
                auto state = wsTransport->getState();
                switch (state) {
                    case WebSocket::InternalState::CONNECTED:
                        sendControlRequest(request, reqListener, tutor);
                        break;
                    case WebSocket::InternalState::CONNECTING:
                        // Se almacenan las solicitudes en búfer, que se enviarán cuando el estado del cliente sea CONNECTED.
                        controlRequestQueue.emplace_back(request, reqListener, tutor);
                        break;
                    default:
                        sessionLog.warn("Unexpected request " + request.getRequestName() + " in state " +
                                        std::to_string(static_cast<int>(state)));
                        break;
                }
            }
        }

        void close(bool waitPending) {
            if (log.IsDebugEnabled) {
                log.Debug("Cerrando WebSocket: " + wsTransport);
            }
            if (wsTransport != nullptr) {
                wsTransport->close();
                wsTransport = nullptr;
            }
        }

        void copyTo(ControlRequestHandler *newHandler) {
            if (ongoingRequest != nullptr) {
                newHandler->addRequest(ongoingRequest->request, ongoingRequest->tutor, ongoingRequest->reqListener);
            }
            for (const auto &pendingRequest: controlRequestQueue) {
                newHandler->addRequest(pendingRequest.request, pendingRequest.tutor, pendingRequest.reqListener);
            }
            // Liberar memoria
            ongoingRequest = nullptr;
            controlRequestQueue.clear();
        }

        /* Method to set the default session ID of a WebSocket connection.
        * The default ID is the one returned in the CONOK response of a bind_session.
        * It remains valid until the receipt of a LOOP or END message.
        */
        void setDefaultSessionId(const std::string &sessionId) {
            Debug.Assert(wsTransport != nullptr);
            wsTransport->setDefaultSessionId(sessionId);
        }

        // Method to find the listener associated with the request.
        // If found, removes it from the list of pending requests.
        transport::RequestListener *getAndRemoveRequestListener(long reqId) {
            RequestListener *reqListener = pendingRequestMap.GetValueOrNull(reqId);
            pendingRequestMap.Remove(reqId);

            return reqListener;
        }


    };

}
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETREQUESTMANAGER_HPP
