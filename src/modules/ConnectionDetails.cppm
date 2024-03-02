/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 2/3/24
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

export module ConnectionDetails;

import <string>;

namespace Lightstreamer::Cpp::ConnectionDetails {
    typedef int AdapterSet;  // TODO:  Define the type of AdapterSet
    typedef std::string Password; // TODO:  Define the type of Password
    typedef std::string User; // TODO:  Define the type of User
    typedef std::string ServerAddress; // TODO:  Define the type of ServerAddress


    class ConnectionDetails {
    public:
        void getAdapterSet() {}

        void getClientIp() {}

        void getServerAddress() {}

        void getServerInstanceAddress() {}

        void getServerSocketName() {}

        void getSessionId() {}

        void getUser() {}

        void setAdapterSet(AdapterSet adapterset) {}

        void setPassword(Password password) {}

        void setServerAddress(ServerAddress serveraddress) {}

        void setUser(User user) {}


    };
}