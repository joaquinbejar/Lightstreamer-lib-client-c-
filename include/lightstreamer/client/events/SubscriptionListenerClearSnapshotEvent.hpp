/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 12/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONLISTENERCLEARSNAPSHOTEVENT_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONLISTENERCLEARSNAPSHOTEVENT_HPP
#include <string>
#include <memory>
#include <lightstreamer/client/ClientMessageListener.hpp>
#include <lightstreamer/client/events/Event.hpp>
#include <utility>
#include <lightstreamer/client/SubscriptionListener.hpp>

namespace lightstreamer::client::events {

    class SubscriptionListenerClearSnapshotEvent : public Event<SubscriptionListener> {
    private:
        int itemPos;
        std::string itemName;

    public:
        SubscriptionListenerClearSnapshotEvent(std::string  itemName, int itemPos)
                : itemName(std::move(itemName)), itemPos(itemPos) {}

        void applyTo(SubscriptionListener& listener) const override {
            listener.onClearSnapshot(itemName, itemPos);
        }
    };

} // namespace lightstreamer::client::events

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONLISTENERCLEARSNAPSHOTEVENT_HPP
