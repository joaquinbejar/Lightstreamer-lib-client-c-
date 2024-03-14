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

        bool statusIs(StreamStatus what) {
            return this->status == what;
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



    protected:
        // Protected methods and utilities

        // Helper function to process messages
        void onProtocolMessage(const std::string &message) {
            // Implementation
        }

        // More helper functions...

    private:
        bool statusIs(StreamStatus queryStatus) {
            return this->status == queryStatus;
        }

        void stopActive(bool force) {
            // Logic to stop active connection/session based on 'force' parameter
            // Placeholder for actual implementation
        }

        void forwardControlResponseError(int errorCode, std::string errorMessage, void* /*Listener type placeholder*/) {
            // Placeholder for handling error forwarding
        }

        static std::regex SUBOK_REGEX;

    };

// Initialize static regex patterns outside the class
    std::regex TextProtocol::SUBOK_REGEX("SUBOK,(\\d+),(\\d+),(\\d+)");
// Further regex initializations...
} // namespace lightstreamer::client::protocol
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOL_HPP
