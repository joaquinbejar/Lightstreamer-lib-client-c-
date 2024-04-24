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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SERVERSESSION_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SERVERSESSION_HPP
#pragma once

#include <memory>
#include <lightstreamer/client/session/Session.hpp>
#include <lightstreamer/client/session/SessionHTTP.hpp>
#include <lightstreamer/client/session/SessionWS.hpp>

namespace lightstreamer::client::session {

    /**
     * Represents a server session.
     *
     * Note: The class Session, notwithstanding the name, does not represent a server session because in general it has
     * a shorter life span than the corresponding server session. Rather it represents the current stream connection
     * (a server session is made of a sequence of stream connections).
     */
    class ServerSession {
    public:
        enum class State {
            OPEN,
            CLOSED
        };

    private:
        State state;
        std::shared_ptr<Session> streamConnection;

    public:
        /**
         * Constructs a server session using the specified stream connection.
         */
        explicit ServerSession(std::shared_ptr<Session> initialStreamConnection)
                : state(State::OPEN), streamConnection(std::move(initialStreamConnection)) {}

        /**
         * Changes the current stream connection.
         */
        void setNewStreamConnection(std::shared_ptr<Session> newStreamConnection) {
            streamConnection = std::move(newStreamConnection);
        }

        /**
         * Checks whether the current stream connection is the same as the specified connection.
         */
        bool isSameStreamConnection(const std::shared_ptr<Session>& tutorStreamConnection) const {
            return streamConnection == tutorStreamConnection;
        }

        /**
         * Checks if the underlying stream connection is using a HTTP transport.
         */
        bool isTransportHttp() const {
            return dynamic_cast<SessionHTTP*>(streamConnection.get()) != nullptr;
        }

        /**
         * Checks if the underlying stream connection is using a WebSocket transport.
         */
        bool isTransportWS() const {
            return dynamic_cast<SessionWS*>(streamConnection.get()) != nullptr;
        }

        /**
         * Returns whether the server session is open.
         */
        bool isOpen() const {
            return state == State::OPEN;
        }

        /**
         * Returns whether the server session is closed.
         */
        bool isClosed() const {
            return state == State::CLOSED;
        }

        /**
         * Closes the server session.
         */
        void close() {
            state = State::CLOSED;
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SERVERSESSION_HPP
