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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CONSTRAINREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CONSTRAINREQUEST_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/requests/ControlRequest.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to change the maximum bandwidth.
     */
    class ConstrainRequest : public ControlRequest {
    private:
        double maxBandwidth;
        /// This field was added to distinguish between requests made by the client
        /// (where requestId equals clientId) and requests made by retransmission algorithm
        /// (where requestId is different from clientId).
        long clientRequestId;

    public:
        /**
         * Constructs a Change-bandwidth request.
         *
         * @param maxBandwidth The maximum bandwidth to request.
         * @param parent If this is a retransmission, `parent` must be the original client request.
         *               If this is a client request, `parent` must be nullptr.
         */
        ConstrainRequest(double maxBandwidth, std::shared_ptr<ConstrainRequest> parent = nullptr)
                : maxBandwidth(maxBandwidth), clientRequestId(parent == nullptr ? this->requestId : parent->getClientRequestId()) {
            this->addParameter("LS_op", "constrain");

            if (maxBandwidth == 0) {
                this->addParameter("LS_requested_max_bandwidth", "unlimited");
            } else if (maxBandwidth > 0) {
                this->addParameter("LS_requested_max_bandwidth", std::to_string(maxBandwidth));
            }
        }

        /**
         * Gets the maximum bandwidth.
         *
         * @return The maximum bandwidth.
         */
        double getMaxBandwidth() const {
            return maxBandwidth;
        }

        /**
         * Gets the client request ID of the original request made by the client.
         * It may be different from `getRequestId()` if this is a retransmission request.
         *
         * @return The client request ID.
         */
        long getClientRequestId() const {
            return clientRequestId;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONSTRAINREQUEST_HPP
