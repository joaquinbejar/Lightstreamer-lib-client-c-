/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 14/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOL_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOL_HPP

#include <iostream>
#include <string>
#include <regex>
#include <map>
#include <vector>
#include <memory>
#include "Logger.h" // Assume a logger class is defined elsewhere
#include "SessionThread.h"
#include "InternalConnectionOptions.h"
#include "ProtocolListener.h"
#include "StreamListener.h"
#include "RequestManager.h"
#include "HttpTransport.h"
#include "ReverseHeartbeatTimer.h"

namespace lightstreamer::client::protocol {
    class TextProtocol {
    public:
        enum class StreamStatus {
            NO_STREAM = 0,
            OPENING_STREAM = 1,
            READING_STREAM = 2,
            STREAM_CLOSED = 3
        };

        static const std::regex SUBOK_REGEX;
        static const std::regex SUBCMD_REGEX;
        static const std::regex UNSUBSCRIBE_REGEX;
        static const std::regex CONSTRAIN_REGEX;
        static const std::regex SYNC_REGEX;
        static const std::regex CLEAR_SNAPSHOT_REGEX;
        static const std::regex END_OF_SNAPSHOT_REGEX;
        static const std::regex OVERFLOW_REGEX;
        static const std::regex CONFIGURATION_REGEX;
        static const std::regex SERVNAME_REGEX;
        static const std::regex CLIENTIP_REGEX;
        static const std::regex PROG_REGEX;
        static const std::regex CONOK_REGEX;
        static const std::regex CONERR_REGEX;
        static const std::regex END_REGEX;
        static const std::regex LOOP_REGEX;

    protected:
        Logger log; // Simplified logging mechanism for C++
        SessionThread sessionThread;
        std::unique_ptr<HttpRequestManager> httpRequestManager;
        ProtocolListener *session = nullptr;
        StreamListener *activeListener = nullptr;
        StreamStatus status = StreamStatus::NO_STREAM;
        long currentProg = 0;
        InternalConnectionOptions options;
        ReverseHeartbeatTimer reverseHeartbeatTimer;
        int objectId;
        HttpTransport httpTransport;

    public:
        TextProtocol(int objectId, std::shared_ptr<SessionThread> thread, InternalConnectionOptions options,
                     std::unique_ptr<HttpTransport> httpTransport)
                : objectId(objectId), sessionThread(thread), options(options), httpTransport(std::move(httpTransport)) {
            if (log.IsDebugEnabled()) {
                log.Debug("New protocol oid=" + std::to_string(this->objectId));
            }
            this->httpRequestManager = std::make_unique<HttpRequestManager>(thread, this, this->httpTransport.get(),
                                                                            this->options, [this](int errorCode,
                                                                                                  std::string errorMessage) {
                        this->log.Error("The server has generated an error. The session will be closed");
                        this->forwardControlResponseError(errorCode, errorMessage, nullptr);
                    });
            this->reverseHeartbeatTimer = ReverseHeartbeatTimer(thread, this->options);
        }

        void setStatus(StreamStatus value, bool forceConnectionClose = false) {
            this->status = value;
            if (this->statusIs(StreamStatus::STREAM_CLOSED) || this->statusIs(StreamStatus::NO_STREAM)) {
                // We now expect the onClose event, but we're not interested in it
                this->stopActive(forceConnectionClose);
            }
        }


        // Returns the InternalConnectionOptions
        // Note: @deprecated notice adapted as a comment for C++
        // This method is meant to be used ONLY as a workaround for iOS implementation, as
        // it requires to send a non Unified API and platform specific event through the
        // ClientListener interface, whose instances can be accessed through the
        // EventDispatcher reference inside the InternalConnectionOptions.
        InternalConnectionOptions getOptions() {
            return options;
        }

        // Stops active listeners or connections, applying force if necessary
        void stopActive(bool force) {
            if (this->activeListener) {
                this->activeListener->disable();
            }
            if (this->activeConnection) {
                this->activeConnection->close(force);
            }
        }

        // Copies pending requests to another Protocol instance
        void copyPendingRequests(std::shared_ptr<TextProtocol> protocol) {
            // Assuming RequestManager has a method to copy requests to another instance's RequestManager
            this->requestManager.copyTo(protocol->getRequestManager());
            // Example of conditional logic based on dynamic type identification, adapted for C++
            if (protocol) {
                // For example, to enable or disable certain functionality for testing
                protocol->currentProg = this->currentProg;
            }
        }

        // Sets the listener for this protocol
        void setListener(std::shared_ptr<ProtocolListener> listener) {
            this->session = listener;
        }

        // Abstract method to dispatch control requests to the transport layer
        virtual void
        sendControlRequest(std::shared_ptr<LightstreamerRequest> request, std::shared_ptr<RequestTutor> tutor,
                           std::shared_ptr<RequestListener> reqListener) = 0;

        // Method to handle reverse heartbeat
        void handleReverseHeartbeat() {
            // Assuming reverseHeartbeatTimer is an object of a class that manages reverse heartbeat timing
            reverseHeartbeatTimer.onChangeInterval();
        }

        // Method to send a force rebind request
        void sendForceRebind(std::shared_ptr<ForceRebindRequest> request, std::shared_ptr<RequestTutor> tutor) {
            // Assuming httpRequestManager is an object that can manage HTTP requests
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass>(this, tutor);
            httpRequestManager.addRequest(request, tutor, reqListener);
        }

        // Method to send a Destroy request
        void sendDestroy(std::shared_ptr<DestroyRequest> request, std::shared_ptr<RequestTutor> tutor) {
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass2>(this, tutor);
            // Assuming httpRequestManager is an object that can manage HTTP requests
            // httpRequestManager.addRequest(request, tutor, reqListener); // Uncomment if httpRequestManager is implemented
            forwardDestroyRequest(request, tutor, reqListener);
        }

        // Abstract method to forward Destroy request to the derived class for custom handling
        virtual void forwardDestroyRequest(std::shared_ptr<DestroyRequest> request, std::shared_ptr<RequestTutor> tutor,
                                           std::shared_ptr<RequestListener> reqListener) = 0;

        // Method to send a Message request
        void sendMessageRequest(std::shared_ptr<MessageRequest> request, std::shared_ptr<RequestTutor> tutor) {
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass3>(this, tutor, request);
            sendControlRequest(request, tutor, reqListener);
        }

        // Method to send a subscription request
        void sendSubscriptionRequest(std::shared_ptr<SubscribeRequest> request, std::shared_ptr<RequestTutor> tutor) {
            if (log.isDebugEnabled()) {
                // Log debug message about the subscription request
                std::cout << "Subscription parameters: " << request->getTransportUnawareQueryString() << std::endl;
            }
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass4>(this, tutor, request);
            try {
                sendControlRequest(request, tutor, reqListener);
            } catch (const std::exception &e) {
                // Log warning message about the exception
                std::cerr << "Something went wrong here: " << e.what() << std::endl;
            }
        }

        void sendConfigurationRequest(std::shared_ptr<ChangeSubscriptionRequest> request,
                                      std::shared_ptr<RequestTutor> tutor) {
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass5>(this, tutor, request);
            sendControlRequest(request, tutor, reqListener);
        }

        void
        sendUnsubscriptionRequest(std::shared_ptr<UnsubscribeRequest> request, std::shared_ptr<RequestTutor> tutor) {
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass6>(this, tutor, request);
            sendControlRequest(request, tutor, reqListener);
        }

        void sendConstrainRequest(std::shared_ptr<ConstrainRequest> request, std::shared_ptr<ConstrainTutor> tutor) {
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass7>(this, tutor, request);
            sendControlRequest(request, tutor, reqListener);
        }

        void
        sendReverseHeartbeat(std::shared_ptr<ReverseHeartbeatRequest> request, std::shared_ptr<RequestTutor> tutor) {
            auto reqListener = std::make_shared<BaseControlRequestListenerAnonymousInnerClass>(this, tutor);
            sendControlRequest(request, tutor, reqListener);
        }

        class BaseControlRequestListener : public RequestListener {
        protected:
            TextProtocol *outerInstance;
            std::shared_ptr<RequestTutor> tutor;

        public:
            BaseControlRequestListener(TextProtocol *outerInstance, std::shared_ptr<RequestTutor> tutor)
                    : outerInstance(outerInstance), tutor(tutor) {
                // Como optimización para evitar reprogramaciones innecesarias de mensajes de latido de corazón inverso
                outerInstance->reverseHeartbeatTimer.onControlRequest();
            }

            virtual void onOK() override {
                // El latido del corazón no se preocupa por REQOK
            }

            virtual void onError(int code, const std::string &message) override {
                // El latido del corazón no se preocupa por REQERR
            }
        };

        void TextProtocol::sendCreateRequest(std::shared_ptr<CreateSessionRequest> request) {
            activeListener = std::make_shared<OpenSessionListener>(this);

            long connectDelay = request->getDelay();
            long readDelay = request->getDelay();
            if (request->isPolling()) {
                readDelay += options->getIdleTimeout();
                connectDelay += options->getPollingInterval();
            }

            // create_session se envía siempre sobre HTTP
            activeConnection = httpRequestManager->createSession(request, activeListener,
                                                                 options->getTCPConnectTimeout() + connectDelay,
                                                                 options->getTCPReadTimeout() + readDelay);

            setStatus(StreamStatus::OPENING_STREAM);
        }

        std::shared_ptr<ListenableFuture> TextProtocol::sendBindRequest(std::shared_ptr<BindSessionRequest> request) {
            activeListener = std::make_shared<BindSessionListener>(this);

            long connectDelay = request->getDelay();
            long readDelay = request->getDelay();
            if (request->isPolling()) {
                readDelay += options->getIdleTimeout();
                connectDelay += options->getPollingInterval();
            }

            auto bindFuture = std::make_shared<ListenableFuture>();

            activeConnection = requestManager->bindSession(request, activeListener,
                                                           options->getTCPConnectTimeout() + connectDelay,
                                                           options->getTCPReadTimeout() + readDelay, bindFuture);

            setStatus(StreamStatus::OPENING_STREAM);
            return bindFuture;
        }

        void TextProtocol::sendRecoveryRequest(std::shared_ptr<RecoverSessionRequest> request) {
            activeListener = std::make_shared<OpenSessionListener>(this);

            long connectDelay = request->getDelay();
            long readDelay = request->getDelay();
            if (request->isPolling()) {
                readDelay += options->getIdleTimeout();
                connectDelay += options->getPollingInterval();
            }

            // La recuperación siempre se envía sobre HTTP
            activeConnection = httpRequestManager->recoverSession(request, activeListener,
                                                                  options->getTCPConnectTimeout() + connectDelay,
                                                                  options->getTCPReadTimeout() + readDelay);

            setStatus(StreamStatus::OPENING_STREAM);
        }

        void onProtocolMessage(const std::string& message) {
            // TODO: Implement the rest of the method
            log::debug("New message (" + std::to_string(objectId) + "): " + message);

            if (status == StreamStatus::READING_STREAM) {
                if (startsWith(message, ProtocolConstants::reqokMarker)) {
                    processREQOK(message);
                } else if (startsWith(message, ProtocolConstants::reqerrMarker)) {
                    processREQERR(message);
                } else if (startsWith(message, ProtocolConstants::errorMarker)) {
                    processERROR(message);
                }
                    // Añade aquí el resto de los casos 'else if' para los diferentes tipos de mensajes...
                else if (startsWith(message, "PROBE")) {
                    session.onKeepalive();
                }
                // Asegúrate de completar todos los casos del switch original, incluyendo SUBOK, UNSUB, CONS, SYNC, etc.
            } else if (status == StreamStatus::OPENING_STREAM) {
                if (startsWith(message, ProtocolConstants::reqokMarker)) {
                    processREQOK(message);
                }
                // Completa con el resto de casos para el estado OPENING_STREAM
            } else if (status == StreamStatus::STREAM_CLOSED) {
                // Manejar mensajes inesperados en estado STREAM_CLOSED
                log::error("Unexpected message in STREAM_CLOSED state: " + message);
            }
            // Considera cualquier otro estado necesario
        }


    protected:
        // Protected methods and utilities

        // Helper function to process messages
        void onProtocolMessage(const std::string &message) {
            // Implementation
        }

        class ControlRequestListener : public RequestListener {
        public:
            virtual void onOK() override = 0;

            virtual void onError(int code, const std::string &message) override = 0;
        };

        // Abstract method for sending control requests, to be implemented by derived classes
        virtual void
        sendControlRequest(std::shared_ptr<LightstreamerRequest> request, std::shared_ptr<RequestTutor> tutor,
                           std::shared_ptr<RequestListener> reqListener) = 0;


    private:
        bool statusIs(StreamStatus queryStatus) {
            return this->status == queryStatus;
        }

        void
        forwardControlResponseError(int errorCode, std::string errorMessage, void * /*Listener type placeholder*/) {
            // Placeholder for handling error forwarding
        }

        class ControlRequestListenerAnonymousInnerClass : public ControlRequestListener {
            TextProtocol *outerInstance;
            std::shared_ptr<RequestTutor> tutor;

        public:
            ControlRequestListenerAnonymousInnerClass(TextProtocol *outerInstance, std::shared_ptr<RequestTutor> tutor)
                    : outerInstance(outerInstance), tutor(tutor) {}

            void onOK() override {
                // Empty implementation, can be customized as needed
            }

            void onError(int code, const std::string &message) override {
                tutor->discard();
                // Log error, assuming a logging mechanism exists
                std::cerr << "force_rebind request caused the error: " << code << " " << message
                          << " - The error will be silently ignored." << std::endl;
            }
        };

        class ControlRequestListenerAnonymousInnerClass2 : public ControlRequestListener {
            TextProtocol *outerInstance;
            std::shared_ptr<RequestTutor> tutor;

        public:
            ControlRequestListenerAnonymousInnerClass2(TextProtocol *outerInstance, std::shared_ptr<RequestTutor> tutor)
                    : outerInstance(outerInstance), tutor(tutor) {}

            void onOK() override {
                // Empty implementation, can be customized as needed
            }

            void onError(int code, const std::string &message) override {
                // Log error, assuming a logging mechanism exists
                std::cerr << "destroy request caused the error: " << code << " " << message
                          << " - The error will be silently ignored." << std::endl;
            }
        };

        class ControlRequestListenerAnonymousInnerClass3 : public ControlRequestListener {
            TextProtocol *outerInstance;
            std::shared_ptr<MessageRequest> request;

        public:
            ControlRequestListenerAnonymousInnerClass3(TextProtocol *outerInstance, std::shared_ptr<RequestTutor> tutor,
                                                       std::shared_ptr<MessageRequest> request)
                    : ControlRequestListener(outerInstance, tutor), request(request) {}

            void onOK() override {
                if (request->needsAck()) {
                    outerInstance->session.onMessageAck(request->getSequence(), request->getMessageNumber(),
                                                        ProtocolConstants::SYNC_RESPONSE);
                } else {
                    // Handling of unneeded acks
                }
            }

            void onError(int code, const std::string &message) override {
                outerInstance->session.onMessageError(request->getSequence(), code, message,
                                                      request->getMessageNumber(), ProtocolConstants::SYNC_RESPONSE);
            }
        };

        class ControlRequestListenerAnonymousInnerClass4 : public ControlRequestListener {
            TextProtocol *outerInstance;
            std::shared_ptr<SubscribeRequest> request;

        public:
            ControlRequestListenerAnonymousInnerClass4(TextProtocol *outerInstance, std::shared_ptr<RequestTutor> tutor,
                                                       std::shared_ptr<SubscribeRequest> request)
                    : outerInstance(outerInstance), request(request) {}

            void onOK() override {
                outerInstance->session.onSubscriptionAck(request->getSubscriptionId());
            }

            void onError(int code, const std::string &message) override {
                outerInstance->session.onSubscriptionError(request->getSubscriptionId(), code, message,
                                                           ProtocolConstants::SYNC_RESPONSE);
            }
        };



    };

    const std::regex TextProtocol::SUBOK_REGEX("SUBOK,(\\d+),(\\d+),(\\d+)");
    const std::regex TextProtocol::SUBCMD_REGEX("SUBCMD,(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");
    const std::regex TextProtocol::UNSUBSCRIBE_REGEX("UNSUB,(\\d+)");
    const std::regex TextProtocol::CONSTRAIN_REGEX("CONS,(unmanaged|unlimited|(\\d+(\\.\\d+)?))");
    const std::regex TextProtocol::SYNC_REGEX("SYNC,(\\d+)");
    const std::regex TextProtocol::CLEAR_SNAPSHOT_REGEX("CS,(\\d+),(\\d+)");
    const std::regex TextProtocol::END_OF_SNAPSHOT_REGEX("EOS,(\\d+),(\\d+)");
    const std::regex TextProtocol::OVERFLOW_REGEX("OV,(\\d+),(\\d+),(\\d+)");
    const std::regex TextProtocol::CONFIGURATION_REGEX("CONF,(\\d+),(unlimited|(\\d+(\\.\\d+)?)),(filtered|unfiltered)");
    const std::regex TextProtocol::SERVNAME_REGEX("SERVNAME,(.+)");
    const std::regex TextProtocol::CLIENTIP_REGEX("CLIENTIP,(.+)");
    const std::regex TextProtocol::PROG_REGEX("PROG,(\\d+)");
    const std::regex TextProtocol::CONOK_REGEX("CONOK,([^,]+),(\\d+),(\\d+),([^,]+)");
    const std::regex TextProtocol::CONERR_REGEX("CONERR,([-]?\\d+),(.*)");
    const std::regex TextProtocol::END_REGEX("END,([-]?\\d+),(.*)");
    const std::regex TextProtocol::LOOP_REGEX("LOOP,(\\d+)");

// Further regex initializations...
} // namespace lightstreamer::client::protocol
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOL_HPP
