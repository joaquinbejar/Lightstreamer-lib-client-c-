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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTHANDLE_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTHANDLE_HPP

namespace lightstreamer::client::transport {

    class RequestHandle {
    public:
        virtual ~RequestHandle() = default;

        /**
         * Suggested implementation is to stop event notifications and to close the
         * associated socket. However, it's also possible to ignore this call, which
         * might be a waste of resources but can help during development.
         *
         * @param forceConnectionClose If true, closes the underlying socket;
         * otherwise, marks the connection as closed but keeps the socket open.
         */
        virtual void close(bool forceConnectionClose) = 0;
    };

} // namespace lightstreamer::client::transport

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTHANDLE_HPP
