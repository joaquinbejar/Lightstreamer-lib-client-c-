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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTLISTENER_HPP

#include <string>

namespace lightstreamer::client::transport {

    class RequestListener {
    public:
        virtual ~RequestListener() = default;

        // Called to notify of new data on the connection.
        virtual void onMessage(const std::string &message) = 0;

        // Called as soon as the socket was opened, and before the request is written on the net.
        virtual void onOpen() = 0;

        // Called at the natural end of the connection after all the received data has been forwarded.
        virtual void onClosed() = 0;

        // Called if the connection is broken before the full response was received.
        virtual void onBroken() = 0;
    };

} // namespace lightstreamer::client::transport

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTLISTENER_HPP
