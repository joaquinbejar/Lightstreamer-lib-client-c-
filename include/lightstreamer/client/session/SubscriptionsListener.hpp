/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 13/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONSLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONSLISTENER_HPP

#include <string>
#include <vector>

namespace lightstreamer::client::session {

    class SubscriptionsListener {
    public:
        virtual ~SubscriptionsListener() = default;

        virtual void onSessionStart() = 0;
        virtual void onSessionClose() = 0;

        virtual void onUpdateReceived(int subscriptionId, int item, const std::vector<std::string>& args) = 0;

        virtual void onEndOfSnapshotEvent(int subscriptionId, int item) = 0;

        virtual void onClearSnapshotEvent(int subscriptionId, int item) = 0;

        virtual void onLostUpdatesEvent(int subscriptionId, int item, int lost) = 0;

        virtual void onUnsubscription(int subscriptionId) = 0;

        virtual void onSubscription(int subscriptionId, int totalItems, int totalFields, int keyPosition, int commandPosition) = 0;

        virtual void onSubscription(int subscriptionId, long long reconfId) = 0;

        virtual void onSubscriptionError(int subscriptionId, int errorCode, const std::string& errorMessage) = 0;

        virtual void onConfigurationEvent(int subscriptionId, const std::string& frequency) = 0;

        virtual void onSubscriptionAck(int subscriptionId) = 0;

        virtual void onUnsubscriptionAck(int subscriptionId) = 0;
    };

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONSLISTENER_HPP
