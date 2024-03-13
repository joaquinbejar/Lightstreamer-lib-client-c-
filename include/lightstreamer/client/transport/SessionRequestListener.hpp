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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONREQUESTLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONREQUESTLISTENER_HPP

#include <lightstreamer/client/transport/RequestListener.hpp>

namespace lightstreamer::client::transport {

    // Listener handling a data connection (in streaming or polling).
    class SessionRequestListener : public RequestListener {
    public:
        ~SessionRequestListener() override = default;

        // No additional methods are defined in SessionRequestListener;
        // it serves as a specialization of RequestListener.
    };

} // namespace lightstreamer::client::transport

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONREQUESTLISTENER_HPP
