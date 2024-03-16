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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOLWS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOLWS_HPP
#include <lightstreamer/client/protocol/TextProtocol.hpp>
#include "WebSocketRequestManager.hpp" // Assuming WebSocketRequestManager is defined
#include <lightstreamer/client/requests/RequestTutor.hpp>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include "RequestListener.hpp" // Ditto
#include "SessionThread.hpp" // Ditto
#include "InternalConnectionOptions.hpp" // Ditto
#include "InternalConnectionDetails.hpp" // Ditto
#include "Http.hpp" // Ditto
#include <lightstreamer/client/protocol/ControlResponseParser.hpp>

#include <string>
#include <memory>
#include <future> // For ListenableFuture equivalent

namespace lightstreamer::client::protocol {

    class TextProtocolWS : public TextProtocol {
    private:
        WebSocketRequestManager wsRequestManager;

    public:
        TextProtocolWS(int objectId, SessionThread& thread, InternalConnectionOptions& options, InternalConnectionDetails& details, Http& httpTransport)
                : TextProtocol(objectId, thread, options, httpTransport), wsRequestManager(thread, *this, options) {}

        // In C++, getter for wsRequestManager acting as RequestManager
        WebSocketRequestManager& getRequestManager() {
            return wsRequestManager;
        }

        std::future<void> openWebSocketConnection(const std::string& serverAddress) {
            return wsRequestManager.openWS(*this, serverAddress, BindSessionListener(*this));
        }

        void sendControlRequest(requests::LightstreamerRequest& request, requests::RequestTutor& tutor, transport::RequestListener& reqListener) override {
            wsRequestManager.addRequest(request, tutor, reqListener);
        }

        void processREQOK(const std::string& message) override {
            try {
                auto parser = REQOKParser(message); // Assuming REQOKParser is a static method or constructible
                auto reqListener = wsRequestManager.getAndRemoveRequestListener(parser.getRequestId());
                if (!reqListener) {
                    // discard the response of a request made outside of the current session
                    logWarn("Acknowledgement discarded: " + message);
                } else {
                    // notify the request listener
                    reqListener->onMessage(message);
                    reqListener->onClosed();
                }
            } catch (const ParsingException& e) {
                onIllegalMessage(e.what());
            }
        }

        void processREQERR(const std::string& message) override {
            try {
                auto parser = REQERRParser(message);
                auto reqListener = wsRequestManager.getAndRemoveRequestListener(parser.requestId);
                if (!reqListener) {
                    // discard the response of a request made outside of the current session
                    logWarn("Acknowledgement discarded: " + message);
                } else {
                    // notify the request listener
                    reqListener->onMessage(message);
                    reqListener->onClosed();
                }
            } catch (const ParsingException& e) {
                onIllegalMessage(e.what());
            }
        }

        void processERROR(const std::string& message) override {
            // Closing the session because of unexpected error
            logError("Closing the session because of unexpected error: " + message);
            try {
                auto parser = ERRORParser(message);
                forwardControlResponseError(parser.errorCode, parser.errorMsg, nullptr);
            } catch (const ParsingException& e) {
                onIllegalMessage(e.what());
            }
        }

        void stop(bool waitPendingControlRequests, bool forceConnectionClose) override {
            logInfo("Stop Protocol");
            TextProtocol::stop(waitPendingControlRequests, forceConnectionClose);
            httpRequestManager.close(waitPendingControlRequests);
            wsRequestManager.close(waitPendingControlRequests);
        }


    protected:
        void onBindSessionForTheSakeOfReverseHeartbeat() override {
            reverseHeartbeatTimer.onBindSession(true);
        }

        void forwardDestroyRequest(DestroyRequest& request, requests::RequestTutor& tutor, transport::RequestListener& reqListener) override {
            wsRequestManager.addRequest(request, tutor, reqListener);
        }

    public:
        // Since C++ doesn't directly support properties as in C#, we use setter (and getter if needed) functions.
        void setDefaultSessionId(const std::string& sessionId) {
            wsRequestManager.setDefaultSessionId(sessionId);
        }


    };

} // namespace com::lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOLWS_HPP
