/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 15/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOL_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOL_HPP

#include <memory>
#include <lightstreamer/client/protocol/ProtocolListener.hpp>
#include "ForceRebindRequest.h"
#include "DestroyRequest.h"
#include "ConstrainRequest.h"
#include "CreateSessionRequest.h"
#include "BindSessionRequest.h"
#include "MessageRequest.h"
#include "SubscribeRequest.h"
#include "UnsubscribeRequest.h"
#include "ChangeSubscriptionRequest.h"
#include "ReverseHeartbeatRequest.h"
#include "RequestManager.h"
#include <lightstreamer/client/requests/RequestTutor.hpp>
#include "ConstrainTutor.hpp"
#include <lightstreamer/util/ListenableFuture.hpp>
#include <string>
#include <exception>

namespace lightstreamer::client::protocol {

    /**
     * @class Protocol
     * @brief The Protocol class is an abstract base class for implementing protocols.
     * It defines the interface for sending requests and managing protocol operations.
     *
     * @details The Protocol class provides methods for sending different types of requests,
     * such as force rebind, destroy, create session, bind session, send message, subscribe,
     * unsubscribe, change subscription, reverse heartbeat, recovery, and configuration requests.
     * It also provides methods for setting listeners, stopping the protocol, handling reverse heartbeat,
     * handling fatal errors, opening WebSocket connections, setting default session ID, getting maximum
     * reverse heartbeat interval, stopping active sessions, and managing request managers.
     *
     * @note All the public methods in this class are pure virtual, and need to be implemented
     * by derived classes.
     */
    class Protocol {
    public:
        virtual ~Protocol() = default;

        virtual void setListener(std::shared_ptr<ProtocolListener> listener) = 0;

        virtual void sendForceRebind(const ForceRebindRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void sendDestroy(const DestroyRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void sendConstrainRequest(const ConstrainRequest& request, const std::shared_ptr<ConstrainTutor>& tutor) = 0;

        virtual void sendCreateRequest(const CreateSessionRequest& request) = 0;

        virtual std::shared_ptr<util::ListenableFuture> sendBindRequest(const BindSessionRequest& request) = 0;

        virtual void stop(bool waitPendingControlRequests, bool forceConnectionClose) = 0;

        virtual void sendMessageRequest(const MessageRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void sendSubscriptionRequest(const SubscribeRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void sendUnsubscriptionRequest(const UnsubscribeRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void sendConfigurationRequest(const ChangeSubscriptionRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void sendReverseHeartbeat(const ReverseHeartbeatRequest& request, const std::shared_ptr<requests::RequestTutor>& tutor) = 0;

        virtual void copyPendingRequests(std::shared_ptr<Protocol> protocol) = 0;

        virtual std::shared_ptr<RequestManager> getRequestManager() const = 0;

        virtual void handleReverseHeartbeat() = 0;

        /// A non-recoverable error causing the closing of the session
        /// and the notification of the error 61 to the method ClientListener::onServerError(int, std::string).
        virtual void onFatalError(const std::exception& cause) = 0;

        /// Opens a WebSocket connection. If a connection is already open (this can happen when the flag isEarlyWSOpenEnabled is set),
        /// the connection is closed and a new connection is opened.
        virtual std::shared_ptr<ListenableFuture> openWebSocketConnection(const std::string& serverAddress) = 0;

        /// Forward the session recovery request to the transport layer.
        virtual void sendRecoveryRequest(const RecoverSessionRequest& request) = 0;

        /// Set the default sessionId so the protocol can omit parameter LS_session from requests.
        virtual void setDefaultSessionId(const std::string& sessionId) = 0;

        /// The maximum time between two heartbeats.
        /// It is the value of the parameter LS_inactivity_millis sent with a bind_session request.
        /// It doesn't change during the life of a session.
        virtual long getMaxReverseHeartbeatIntervalMs() const = 0;

        virtual void sendRecoveryRequest(const RecoverSessionRequest& request) = 0;

        virtual void stopActive(bool forceConnectionClose) = 0;
    };

} // namespace lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOL_HPP
