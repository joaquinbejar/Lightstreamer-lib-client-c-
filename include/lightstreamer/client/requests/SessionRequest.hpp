/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 31/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONREQUEST_HPP

#include <string>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>

namespace lightstreamer::client::requests {

    class SessionRequest : public LightstreamerRequest {
    protected:
        bool polling;
        long delay;

    public:
        SessionRequest(bool polling, long delay)
                : polling(polling), delay(delay) {}

        // C++ doesn't support properties directly, so use getter and setter methods
        bool isPolling() const {
            return polling;
        }

        void setPolling(bool newPolling) {
            polling = newPolling;
        }

        long getDelay() const {
            return delay;
        }

        void setDelay(long newDelay) {
            delay = newDelay;
        }

        // Assuming RequestName is a required implementation from LightstreamerRequest
        std::string getRequestName() const override {
            // C++ doesn't support returning null for std::string, so return an empty string instead
            return "";
        }
    };

} // namespace lightstreamer::client::requests


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONREQUEST_HPP
