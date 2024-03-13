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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_MESSAGESLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_MESSAGESLISTENER_HPP


#include <string>

namespace lightstreamer::client::session {

    class MessagesListener {
    public:
        virtual ~MessagesListener() = default;

        virtual void onSessionStart() = 0;
        virtual void onSessionClose() = 0;

        virtual void onMessageAck(const std::string& sequence, int number) = 0;

        virtual void onMessageOk(const std::string& sequence, int number) = 0;

        virtual void onMessageDeny(const std::string& sequence, int denyCode, const std::string& denyMessage, int number) = 0;

        virtual void onMessageDiscarded(const std::string& sequence, int number) = 0;

        virtual void onMessageError(const std::string& sequence, int errorCode, const std::string& errorMessage, int number) = 0;
    };

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_MESSAGESLISTENER_HPP
