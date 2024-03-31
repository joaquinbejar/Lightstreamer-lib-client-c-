/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 31/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_UNSUBSCRIBEREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_UNSUBSCRIBEREQUEST_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/requests/ControlRequest.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to unsubscribe from a subscription.
     */
    class UnsubscribeRequest : public ControlRequest {
    private:
        int subscriptionId;

    public:
        /**
         * Constructs an UnsubscribeRequest with a given subscription ID.
         *
         * @param subId The subscription ID to unsubscribe from.
         */
        UnsubscribeRequest(int subId) : subscriptionId(subId) {
            this->addParameter("LS_op", "delete");
            this->addParameter("LS_subId", std::to_string(subId));
        }

        /**
         * Gets the subscription ID associated with this unsubscribe request.
         *
         * @return The subscription ID.
         */
        int getSubscriptionId() const {
            return subscriptionId;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_UNSUBSCRIBEREQUEST_HPP
