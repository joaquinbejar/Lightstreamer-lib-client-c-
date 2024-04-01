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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_FORCEREBINDREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_FORCEREBINDREQUEST_HPP

#include <memory>
#include <string>
#include <lightstreamer/client/requests/ControlRequest.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to forcibly rebind the current session with specified parameters.
     */
    class ForceRebindRequest : public ControlRequest {
    public:
        /**
         * Constructs a ForceRebindRequest with specified parameters.
         *
         * @param targetServer The target server for the rebind operation.
         * @param sessionID The current session ID.
         * @param rebindCause The cause for rebinding.
         * @param delay The delay before attempting the rebind, in milliseconds.
         */
        ForceRebindRequest(const std::string &targetServer, const std::string &sessionID,
                           const std::string &rebindCause, double delay) {
            this->Server = targetServer;

            this->addParameter("LS_op", "force_rebind");
            this->addParameter("LS_session", sessionID);

            if (!rebindCause.empty()) {
                this->addParameter("LS_cause", rebindCause);
            }

            if (delay > 0) {
                this->addParameter("LS_polling_millis", std::to_string(delay));
            }
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_FORCEREBINDREQUEST_HPP
