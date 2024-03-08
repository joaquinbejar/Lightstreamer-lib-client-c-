/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 8/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTLISTENER_HPP

namespace lightstreamer::client {
    class ClientListener {
    public:
        virtual void onListenEnd(LightstreamerClient* client) = 0;
        virtual void onListenStart(LightstreamerClient* client) = 0;
        virtual void onServerError(int errorCode, const std::string& errorMessage) = 0;
        virtual void onStatusChange(const std::string& status) = 0;
        virtual void onPropertyChange(const std::string& property) = 0;
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CLIENTLISTENER_HPP
