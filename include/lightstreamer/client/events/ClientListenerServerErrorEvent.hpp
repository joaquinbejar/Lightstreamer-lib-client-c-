/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 8/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTLISTENERSERVERERROREVENT_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTLISTENERSERVERERROREVENT_HPP

#include <string>
#include <lightstreamer/client/events/Event.hpp>
#include <lightstreamer/client/ClientListener.hpp>

namespace lightstreamer::client::events {
    class ClientListenerServerErrorEvent : public Event<ClientListener> {
    private:
        std::string errorMessage;
        int errorCode;

    public:
        ClientListenerServerErrorEvent(int errorCode, const std::string &errorMessage) {
            this->errorCode = errorCode;
            this->errorMessage = errorMessage;
        }

        void applyTo(ClientListener &listener) const override {
            listener.onServerError(errorCode, errorMessage);
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTLISTENERSERVERERROREVENT_HPP
