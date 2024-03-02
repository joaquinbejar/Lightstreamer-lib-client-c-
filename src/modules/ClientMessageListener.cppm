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

export module ClientMessageListener;

import <string>;

namespace Lightstreamer::Cpp::ClientMessageListener {
    typedef std::string Mesagge; // TODO: Define the message type
    typedef bool SentOnNetwork; // TODO: Define the message type
    typedef int Code; // TODO: Define the code type
    typedef std::string Error; // TODO: Define the error type

    class ClientMessageListener {
    public:
        void onAbort(Mesagge &msg, SentOnNetwork sentOnNetwork) {}

        void onDiscarded(Mesagge msg) {}

        void onError(Mesagge msg) {}

        void onProcessed(Mesagge msg) {}

        void onDeny(Mesagge msg, Code code, Error error) {}


    };

}