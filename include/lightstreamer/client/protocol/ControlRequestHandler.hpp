/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLREQUESTHANDLER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLREQUESTHANDLER_HPP
#ifndef CONTROLREQUESTHANDLER_H
#define CONTROLREQUESTHANDLER_H

#include <memory>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/client/requests/RequestTutor.hpp>
#include <lightstreamer/client/transport/RequestListener.hpp>

namespace lightstreamer::client::protocol {

/**
 * Interface for handling control/message requests.
 */
    class ControlRequestHandler {
    public:
        virtual ~ControlRequestHandler() = default;

        /**
         * Adds a control/message request.
         *
         * @param request The Lightstreamer request to be added.
         * @param tutor The tutor associated with the request.
         * @param reqListener The request listener.
         */
        virtual void addRequest(std::shared_ptr<requests::LightstreamerRequest> request,
                                std::shared_ptr<requests::RequestTutor> tutor,
                                std::shared_ptr<transport::RequestListener> reqListener) = 0;

        /**
         * Sets the request limit.
         *
         * @param limit The request limit.
         */
        virtual void setRequestLimit(long limit) = 0;

        /**
         * Copies the handler's state to a new handler.
         *
         * @param newHandler The new handler to copy the state to.
         */
        virtual void copyTo(std::shared_ptr<ControlRequestHandler> newHandler) = 0;

        /**
         * Closes the handler.
         *
         * @param waitPending Whether to wait for pending requests before closing.
         */
        virtual void close(bool waitPending) = 0;
    };

} // namespace lightstreamer::client::protocol

#endif // CONTROLREQUESTHANDLER_H

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLREQUESTHANDLER_HPP
