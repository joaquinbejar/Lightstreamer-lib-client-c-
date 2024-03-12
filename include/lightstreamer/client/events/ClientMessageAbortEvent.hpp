/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 11/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTMESSAGEABORTEVENT_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTMESSAGEABORTEVENT_HPP

#include <utility>
#include <string>
#include <lightstreamer/client/ClientListener.hpp>
#include <lightstreamer/client/ClientMessageListener.hpp>
#include <lightstreamer/client/LightstreamerClient.hpp>
#include <lightstreamer/client/events/Event.hpp>

namespace lightstreamer::client::events {

    class ClientMessageAbortEvent : public Event<ClientMessageListener> {
    private:
        std::string originalMessage;
        bool sentOnNetwork;

    public:
        ClientMessageAbortEvent(std::string originalMessage, bool sentOnNetwork)
                : originalMessage(std::move(originalMessage)), sentOnNetwork(sentOnNetwork) {}

        void applyTo(ClientMessageListener &listener) const override {
            listener.onAbort(originalMessage, sentOnNetwork);
        }
    };

} // namespace lightstreamer::client::events


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTMESSAGEABORTEVENT_HPP
