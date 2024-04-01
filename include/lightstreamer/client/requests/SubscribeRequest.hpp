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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIBEREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIBEREQUEST_HPP

#include <memory>
#include <string>
#include <lightstreamer/client/requests/ControlRequest.hpp>
#include <lightstreamer/util/Descriptor.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to subscribe to items with specified criteria.
     */
    class SubscribeRequest : public ControlRequest {
    private:
        int id;

    public:
        /**
         * Constructs a SubscribeRequest with subscription parameters.
         *
         * @param subId The subscription ID.
         * @param mode The subscription mode.
         * @param items The descriptor for the items to subscribe to.
         * @param fields The descriptor for the fields to subscribe to.
         * @param dataAdapter The data adapter name.
         * @param selector The selector name.
         * @param requiredSnapshot Indicates if a snapshot is required.
         * @param requestedMaxFrequency The maximum update frequency requested.
         * @param requestedBufferSize The buffer size requested.
         */
        SubscribeRequest(int subId, const std::string &mode, const util::Descriptor &items,
                         const util::Descriptor &fields, const std::string &dataAdapter, const std::string &selector,
                         const std::string &requiredSnapshot, double requestedMaxFrequency, int requestedBufferSize)
                : id(subId) {
            addParameter("LS_op", "add");
            addParameter("LS_subId", std::to_string(subId));

            addParameter("LS_mode", mode);
            addParameter("LS_group", items.ComposedString());
            addParameter("LS_schema", fields.ComposedString());

            if (!dataAdapter.empty()) {
                addParameter("LS_data_adapter", dataAdapter);
            }

            if (!selector.empty()) {
                addParameter("LS_selector", selector);
            }

            if (!requiredSnapshot.empty()) {
                if (requiredSnapshot == "yes") {
                    addParameter("LS_snapshot", "true");
                } else if (requiredSnapshot == "no") {
                    addParameter("LS_snapshot", "false");
                } else {
                    addParameter("LS_snapshot", requiredSnapshot);
                }
            }

            if (requestedMaxFrequency == -2) {
                // server default: just omit the parameter
            } else if (requestedMaxFrequency == -1) {
                addParameter("LS_requested_max_frequency", "unfiltered");
            } else if (requestedMaxFrequency == 0) {
                addParameter("LS_requested_max_frequency", "unlimited");
            } else if (requestedMaxFrequency > 0) {
                addParameter("LS_requested_max_frequency", std::to_string(requestedMaxFrequency));
            }

            if (requestedBufferSize == -1) {
                // server default: just omit the parameter
            } else if (requestedBufferSize == 0) {
                addParameter("LS_requested_buffer_size", "unlimited");
            } else if (requestedBufferSize > 0) {
                addParameter("LS_requested_buffer_size", std::to_string(requestedBufferSize));
            }

            //LS_start & LS_end are obsolete, removed from APIs
        }

        /**
         * Gets the subscription ID.
         *
         * @return The subscription ID.
         */
        int getSubscriptionId() const {
            return id;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIBEREQUEST_HPP
