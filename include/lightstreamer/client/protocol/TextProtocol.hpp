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
        TextProtocol(int objectId, std::shared_ptr<SessionThread> thread, InternalConnectionOptions options, std::unique_ptr<HttpTransport> httpTransport)
                : objectId(objectId), sessionThread(thread), options(options), httpTransport(std::move(httpTransport)) {
            if (log.IsDebugEnabled()) {
                log.Debug("New protocol oid=" + std::to_string(this->objectId));
            }
            this->httpRequestManager = std::make_unique<HttpRequestManager>(thread, this, this->httpTransport.get(), this->options, [this](int errorCode, std::string errorMessage) {
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
        virtual void sendControlRequest(std::shared_ptr<LightstreamerRequest> request, std::shared_ptr<RequestTutor> tutor, std::shared_ptr<RequestListener> reqListener) = 0;

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
        virtual void forwardDestroyRequest(std::shared_ptr<DestroyRequest> request, std::shared_ptr<RequestTutor> tutor, std::shared_ptr<RequestListener> reqListener) = 0;

        // Method to send a Message request
        void sendMessageRequest(std::shared_ptr<MessageRequest> request, std::shared_ptr<RequestTutor> tutor) {
            auto reqListener = std::make_shared<ControlRequestListenerAnonymousInnerClass3>(this, tutor, request);
            sendControlRequest(request, tutor, reqListener);
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
            virtual void onError(int code, const std::string& message) override = 0;
        };



    private:
        bool statusIs(StreamStatus queryStatus) {
            return this->status == queryStatus;
        }


        void forwardControlResponseError(int errorCode, std::string errorMessage, void* /*Listener type placeholder*/) {
            // Placeholder for handling error forwarding
        }

        class ControlRequestListenerAnonymousInnerClass : public ControlRequestListener {
            TextProtocol* outerInstance;
            std::shared_ptr<RequestTutor> tutor;

        public:
            ControlRequestListenerAnonymousInnerClass(TextProtocol* outerInstance, std::shared_ptr<RequestTutor> tutor) : outerInstance(outerInstance), tutor(tutor) {}

            void onOK() override {
                // Empty implementation, can be customized as needed
            }

            void onError(int code, const std::string& message) override {
                tutor->discard();
                // Log error, assuming a logging mechanism exists
                std::cerr << "force_rebind request caused the error: " << code << " " << message << " - The error will be silently ignored." << std::endl;
            }
        };
        class ControlRequestListenerAnonymousInnerClass2 : public ControlRequestListener {
            TextProtocol* outerInstance;
            std::shared_ptr<RequestTutor> tutor;

        public:
            ControlRequestListenerAnonymousInnerClass2(TextProtocol* outerInstance, std::shared_ptr<RequestTutor> tutor) : outerInstance(outerInstance), tutor(tutor) {}

            void onOK() override {
                // Empty implementation, can be customized as needed
            }

            void onError(int code, const std::string& message) override {
                // Log error, assuming a logging mechanism exists
                std::cerr << "destroy request caused the error: " << code << " " << message << " - The error will be silently ignored." << std::endl;
            }
        };

        class ControlRequestListenerAnonymousInnerClass3 : public ControlRequestListener {
            TextProtocol* outerInstance;
            std::shared_ptr<MessageRequest> request;

        public:
            ControlRequestListenerAnonymousInnerClass3(TextProtocol* outerInstance, std::shared_ptr<RequestTutor> tutor, std::shared_ptr<MessageRequest> request) : ControlRequestListener(outerInstance, tutor), request(request) {}

            void onOK() override {
                if (request->needsAck()) {
                    outerInstance->session.onMessageAck(request->getSequence(), request->getMessageNumber(), ProtocolConstants::SYNC_RESPONSE);
                } else {
                    // Handling of unneeded acks
                }
            }

            void onError(int code, const std::string& message) override {
                outerInstance->session.onMessageError(request->getSequence(), code, message, request->getMessageNumber(), ProtocolConstants::SYNC_RESPONSE);
            }
        };

        static std::regex SUBOK_REGEX;

    };

// Initialize static regex patterns outside the class
    std::regex TextProtocol::SUBOK_REGEX("SUBOK,(\\d+),(\\d+),(\\d+)");
// Further regex initializations...
} // namespace lightstreamer::client::protocol
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOL_HPP
