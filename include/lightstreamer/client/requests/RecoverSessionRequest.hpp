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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_RECOVERSESSIONREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_RECOVERSESSIONREQUEST_HPP

#include <memory>
#include <string>
#include <lightstreamer/client/requests/SessionRequest.hpp>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>

namespace lightstreamer::client::requests {

    /**
     * A recovery request is a special type of bind_session request with the additional LS_recovery_from parameter.
     * The class was adapted from CreateSessionRequest.
     */
    class RecoverSessionRequest : public SessionRequest {
    public:
        /**
         * Constructs a RecoverSessionRequest with specified parameters for session recovery.
         *
         * @param targetServer The target server address.
         * @param session The session identifier.
         * @param cause The cause of the recovery request.
         * @param options The internal connection options.
         * @param delay The delay before attempting recovery.
         * @param sessionRecoveryProg The session recovery progression identifier.
         */
        RecoverSessionRequest(const std::string &targetServer, const std::string &session, const std::string &cause,
                              std::shared_ptr<session::InternalConnectionOptions> options, long delay,
                              long sessionRecoveryProg)
                : SessionRequest(true, delay) {
            this->Server = targetServer;

            this->addParameter("LS_polling", "true");

            if (!cause.empty()) {
                this->addParameter("LS_cause", cause);
            }

            long requestedPollingInterval =
                    delay > 0 ? delay : 0; // NB: delay can be negative since it is computed by SlowingHandler.
            long requestedIdleTimeout = 0;
            this->addParameter("LS_polling_millis", std::to_string(requestedPollingInterval));
            this->addParameter("LS_idle_millis", std::to_string(requestedIdleTimeout));

            if (options->InternalMaxBandwidth == 0) {
                // unlimited: just omit the parameter
            } else if (options->InternalMaxBandwidth > 0) {
                this->addParameter("LS_requested_max_bandwidth", std::to_string(options->InternalMaxBandwidth));
            }

            this->Session = session;

            this->addParameter("LS_recovery_from", std::to_string(sessionRecoveryProg));
        }

        /**
         * Gets the name of the request.
         *
         * @return Always returns "bind_session".
         */
        std::string getRequestName() const override {
            return "bind_session";
        }

        /**
         * Indicates if this is a session request.
         *
         * @return Always true for RecoverSessionRequest.
         */
        bool isSessionRequest() const override {
            return true;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_RECOVERSESSIONREQUEST_HPP
