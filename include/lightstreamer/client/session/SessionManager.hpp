/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 18/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/session/SessionListener.hpp>
#include <lightstreamer/client/session/SessionsListener.hpp>
#include <lightstreamer/client/session/SessionFactory.hpp>
#include <lightstreamer/client/session/Session.hpp>
#include <lightstreamer/client/session/ServerSession.hpp>
#include <Logger.hpp>


namespace lightstreamer::client::session {

    /**
     * Manages sessions, implementing the SessionListener interface.
     */
    class SessionManager : public SessionListener {

        static std::shared_ptr<SessionFactory> sessionFactory;
    public:
        /**
         * Allows setting a custom session factory.
         * Note: This setter is intended just for testing.
         */
        static void setCustomFactory(std::shared_ptr<SessionFactory> value) {
            sessionFactory = value;
        }

        enum class Status {
            OFF,
            STREAMING_WS,
            SWITCHING_STREAMING_WS,
            POLLING_WS,
            SWITCHING_POLLING_WS,
            STREAMING_HTTP,
            SWITCHING_STREAMING_HTTP,
            POLLING_HTTP,
            SWITCHING_POLLING_HTTP,
            END,
            ERROR
        };

        static constexpr bool STREAMING_SESSION = false;
        static constexpr bool POLLING_SESSION = true;
        static constexpr bool WS_SESSION = false;
        static constexpr bool HTTP_SESSION = true;

        static constexpr bool AVOID_SWITCH = true;
        //static constexpr bool NO_RECOVERY = true;
        static constexpr bool YES_RECOVERY = false;

        /**
         * Converts a status enum to a descriptive string.
         */
        static std::string statusToString(Status type) {
            if (type == Status::ERROR) {
                return {};
            }

            switch (type) {
                case Status::OFF:
                    return "No session";
                case Status::STREAMING_WS:
                    return "WS Streaming";
                case Status::SWITCHING_STREAMING_WS:
                    return "prepare WS Streaming";
                case Status::POLLING_WS:
                    return "WS Polling";
                case Status::SWITCHING_POLLING_WS:
                    return "prepare WS Polling";
                case Status::STREAMING_HTTP:
                    return "HTTP Streaming";
                case Status::SWITCHING_STREAMING_HTTP:
                    return "prepare HTTP Streaming";
                case Status::POLLING_HTTP:
                    return "HTTP Polling";
                case Status::SWITCHING_POLLING_HTTP:
                    return "prepare HTTP Polling";
                case Status::END:
                    return "Shutting down";
                default:
                    return "Error";
            }
        }

        std::shared_ptr<ILogger> log;
        Status status = Status::OFF;
        int statusPhase = 0;
        std::shared_ptr<Session> session;
        std::shared_ptr<ServerSession> serverSession;
        bool isFrozen = false;
        std::string clientIP;
        std::shared_ptr<InternalConnectionOptions> options;
        std::shared_ptr<InternalConnectionDetails> details;
        std::shared_ptr<SubscriptionsListener> subscriptions;
        std::shared_ptr<MessagesListener> messages;
        std::shared_ptr<SessionsListener> listener;
        SessionThread thread;

        /**
         * Counts the bind_session requests following the corresponding create_session.
         */
        int nBindAfterCreate = 0;

    };

    std::shared_ptr<SessionFactory> SessionManager::sessionFactory = std::make_shared<SessionFactory>();
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
