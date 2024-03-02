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

export module ClientListener;
import <string>;

namespace Lightstreamer::Cpp::ClientListener {
    typedef int Property; // TODO: Define the property type
    typedef int Code; // TODO: Define the code type
    typedef std::string Mesagge; // TODO: Define the message type
    typedef int Status; // TODO: Define the status type

    class ClientListener {
    public:
        void onListenEnd() {}

        void onListenStart() {}

        void onPropertyChange(Property property) {}

        void onServerError(Code code, Mesagge message) {}

        void onStatusChange(Status status) {}

    };
}