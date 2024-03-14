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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOLHTTP_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOLHTTP_HPP
#pragma once

#include <lightstreamer/client/protocol/TextProtocol.hpp>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include "RequestTutor.hpp"
#include "Http.hpp" // Assuming Http is a class representing HTTP transport
#include <lightstreamer/util/ListenableFuture.hpp>
#include <cassert>
#include <memory>
#include <lightstreamer/client/transport/RequestListener.hpp>


namespace lightstreamer::client::protocol {

    class TextProtocolHttp : public TextProtocol {
    public:
        TextProtocolHttp(int objectId, SessionThread& thread, InternalConnectionOptions& options, Http& httpTransport)
                : TextProtocol(objectId, thread, options, httpTransport) {
            // Constructor implementation, possibly empty if all initialization happens in the base class
        }

        RequestManager& getRequestManager() override {
            return this->httpRequestManager;
        }

        void sendControlRequest(requests::LightstreamerRequest& request, RequestTutor& tutor, transport::RequestListener& reqListener) override {
            httpRequestManager.addRequest(request, tutor, reqListener);
        }

        void processREQOK(const std::string& message) override {
            assert(false); // Use C++ assert
        }

        void processREQERR(const std::string& message) override {
            assert(false); // Use C++ assert
        }

        void processERROR(const std::string& message) override {
            assert(false); // Use C++ assert
        }

        void stop(bool waitPendingControlRequests, bool forceConnectionClose) override {
            TextProtocol::stop(waitPendingControlRequests, forceConnectionClose);
            this->httpRequestManager.close(waitPendingControlRequests);
        }

        std::shared_ptr<util::ListenableFuture> openWebSocketConnection(const std::string& serverAddress) override {
            // This method should never be called in this class, as stated
            assert(false);
            return util::ListenableFuture::rejected(); // TODO: Adjust according to your ListenableFuture implementation
        }

    protected:
        void onBindSessionForTheSakeOfReverseHeartbeat() override {
            reverseHeartbeatTimer.onBindSession(false);
        }

        void forwardDestroyRequest(DestroyRequest& request, RequestTutor& tutor, transport::RequestListener& reqListener) override {
            // Don't send destroy request when transport is http
        }

        void setDefaultSessionId(const std::string& sessionId) override {
            // HTTP connections don't have a default session id
        }
    };

} // namespace com::lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TEXTPROTOCOLHTTP_HPP
