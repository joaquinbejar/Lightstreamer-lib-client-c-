/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 17/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONHTTP_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONHTTP_HPP

#include "Session.hpp"
#include <memory>
#include <string>
#include <lightstreamer/client/Constants.hpp>

namespace lightstreamer::client::session {

    class SessionHTTP : public Session {
    public:
        SessionHTTP(int objectId, bool isPolling, bool forced, std::shared_ptr<SessionListener> handler,
                    std::shared_ptr<SubscriptionsListener> subscriptions, std::shared_ptr<MessagesListener> messages,
                    std::shared_ptr<Session> originalSession, std::shared_ptr<SessionThread> thread, Protocol protocol,
                    InternalConnectionDetails details, InternalConnectionOptions options, int callerPhase,
                    bool retryAgainIfStreamFails, bool sessionRecovery)
                : Session(objectId, isPolling, forced, handler, subscriptions, messages, originalSession, thread,
                          protocol, details, options, callerPhase, retryAgainIfStreamFails, sessionRecovery) {}

        // Assuming getters and setters for properties if needed

    protected:
        virtual std::string getConnectedHighLevelStatus() const override {
            return isPolling ? Constants::HTTP_POLLING : Constants::HTTP_STREAMING;
        }

        virtual std::string getFirstConnectedStatus() const override {
            return isPolling ? Constants::HTTP_POLLING : Constants::SENSE;
        }

        virtual bool shouldAskContentLength() const override {
            return !isPolling;
        }

    public:
        virtual void sendReverseHeartbeat(std::shared_ptr<ReverseHeartbeatRequest> request,
                                          std::shared_ptr<RequestTutor> tutor) override {
            request->addUnique();
            Session::sendReverseHeartbeat(request, tutor);
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONHTTP_HPP
