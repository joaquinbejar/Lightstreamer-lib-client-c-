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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONLISTENER_HPP

#include <string>
#include "ItemUpdate.hpp" // Assuming ItemUpdate is a class that encapsulates an item update.
#include "Subscription.hpp" // Assuming Subscription is a class that represents a subscription.

namespace lightstreamer::client {

    class SubscriptionListener {
    public:
        virtual ~SubscriptionListener() = default;

        virtual void onClearSnapshot(const std::string& itemName, int itemPos) = 0;
        virtual void onCommandSecondLevelItemLostUpdates(int lostUpdates, const std::string& key) = 0;
        virtual void onCommandSecondLevelSubscriptionError(int code, const std::string& message, const std::string& key) = 0;
        virtual void onEndOfSnapshot(const std::string& itemName, int itemPos) = 0;
        virtual void onItemLostUpdates(const std::string& itemName, int itemPos, int lostUpdates) = 0;
        virtual void onItemUpdate(const ItemUpdate& itemUpdate) = 0;
        virtual void onListenEnd(const Subscription& subscription) = 0;
        virtual void onListenStart(const Subscription& subscription) = 0;
        virtual void onSubscription() = 0;
        virtual void onSubscriptionError(int code, const std::string& message) = 0;
        virtual void onUnsubscription() = 0;
        virtual void onRealMaxFrequency(const std::string& frequency) = 0;
    };

} // namespace lightstreamer::client

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONLISTENER_HPP
