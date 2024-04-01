/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 1/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CHANGESUBSCRIPTIONREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CHANGESUBSCRIPTIONREQUEST_HPP
#include <lightstreamer/client/requests/ControlRequest.hpp>
#include <cassert>
#include <string>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to change subscription parameters.
     */
    class ChangeSubscriptionRequest : public ControlRequest {
    private:
        int reconfId;
        int subscriptionId;

    public:
        /**
         * Constructs a ChangeSubscriptionRequest with specified parameters.
         *
         * @param subscriptionId The ID of the subscription to be changed.
         * @param requestedMaxFrequency The requested maximum update frequency for the subscription.
         * @param reconfId The reconfiguration identifier.
         */
        ChangeSubscriptionRequest(int subscriptionId, double requestedMaxFrequency, int reconfId)
                : reconfId(reconfId), subscriptionId(subscriptionId) {
            this->addParameter("LS_op", "reconf");
            this->addParameter("LS_subId", std::to_string(subscriptionId));

            assert(requestedMaxFrequency != -2);
            assert(requestedMaxFrequency != -1);
            if (requestedMaxFrequency == 0) {
                this->addParameter("LS_requested_max_frequency", "unlimited");
            } else if (requestedMaxFrequency > 0) {
                this->addParameter("LS_requested_max_frequency", std::to_string(requestedMaxFrequency));
            }
        }

        /**
         * Gets the reconfiguration ID.
         *
         * @return The reconfiguration ID.
         */
        int getReconfId() const {
            return reconfId;
        }

        /**
         * Gets the subscription ID.
         *
         * @return The subscription ID.
         */
        int getSubscriptionId() const {
            return subscriptionId;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CHANGESUBSCRIPTIONREQUEST_HPP
