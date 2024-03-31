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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CREATESESSIONREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CREATESESSIONREQUEST_HPP

#include <memory>
#include <string>
#include <lightstreamer/client/requests/SessionRequest.hpp>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>
#include <lightstreamer/client/session/InternalConnectionDetails.hpp>
#include "Logger.hpp" // Adjust to your logging framework.

namespace lightstreamer::client::requests {

    /**
     * Represents a request to create a new session.
     */
    class CreateSessionRequest : public SessionRequest {
    protected:
        // Assuming a logger exists in your project's structure.
        static inline Logger log = LogManager::GetLogger(Constants::PROTOCOL_LOG);

    public:
        /**
         * Constructs a CreateSessionRequest with specified parameters.
         *
         * @param targetServer The target server for the session.
         * @param polling Indicates if polling is used.
         * @param cause The cause for the session creation request.
         * @param options The internal connection options.
         * @param details The internal connection details.
         * @param delay The delay before attempting session creation.
         * @param password The password for authentication.
         * @param oldSession The previous session ID, if any.
         */
        CreateSessionRequest(const std::string &targetServer, bool polling,
                             const std::string &cause, std::shared_ptr<session::InternalConnectionOptions> options,
                             std::shared_ptr<session::InternalConnectionDetails> details, long delay,
                             const std::string &password, const std::string &oldSession)
                : SessionRequest(polling, delay) {
            this->Server = targetServer;

            this->addParameter("LS_polling", polling ? "true" : "false");

            if (!cause.empty()) {
                this->addParameter("LS_cause", cause);
            }

            long requestedPollingInterval = 0;
            long requestedIdleTimeout = 0;
            if (polling) {
                requestedPollingInterval = options->PollingInterval + delay;
            }

            this->addParameter("LS_polling_millis", std::to_string(requestedPollingInterval));
            this->addParameter("LS_idle_millis", std::to_string(requestedIdleTimeout));

            this->addParameter("LS_cid", "jqWtj1twChtfDxikwp1ltvcB4CJ5M5iwVztxHfDprfc7Do");

            if (options->InternalMaxBandwidth > 0) {
                this->addParameter("LS_requested_max_bandwidth", std::to_string(options->InternalMaxBandwidth));
            }

            if (!details->AdapterSet.empty()) {
                this->addParameter("LS_adapter_set", details->AdapterSet);
            }

            if (!details->User.empty()) {
                this->addParameter("LS_user", details->User);
            }

            if (!password.empty()) {
                this->addParameter("LS_password", password);
            }

            if (!oldSession.empty()) {
                this->addParameter("LS_old_session", oldSession);
            }

            log.Debug("Create Request: " + this->Server);
        }

        /**
         * Gets the name of the request.
         *
         * @return Always returns "create_session".
         */
        std::string getRequestName() const override {
            return "create_session";
        }

        /**
         * Indicates if this is a session request.
         *
         * @return Always true for CreateSessionRequest.
         */
        bool isSessionRequest() const override {
            return true;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CREATESESSIONREQUEST_HPP
