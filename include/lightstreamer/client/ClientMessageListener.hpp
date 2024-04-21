/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 11/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTMESSAGELISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTMESSAGELISTENER_HPP

#include <string>

namespace lightstreamer::client {

    class ClientMessageListener {
    public:
        virtual ~ClientMessageListener() = default;

        /// Notification that the related message has been aborted and its outcome is no longer expected.
        virtual void onAbort(const std::string &originalMessage, bool sentOnNetwork) = 0;

        /// Notification that the related message has been denied by the server.
        virtual void onDeny(const std::string &originalMessage, int code, const std::string &error) = 0;

        /// Notification that the related message has been discarded by the server.
        virtual void onDiscarded(const std::string &originalMessage) = 0;

        /// Notification that an error occurred while processing the related message.
        virtual void onError(const std::string &originalMessage) = 0;

        /// Notification that the related message has been successfully processed.
        virtual void onProcessed(const std::string &originalMessage) = 0;
    };

} // namespace lightstreamer::client


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTMESSAGELISTENER_HPP
