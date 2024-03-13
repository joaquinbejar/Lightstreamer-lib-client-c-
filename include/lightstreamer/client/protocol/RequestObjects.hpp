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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTOBJECTS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTOBJECTS_HPP
#pragma once

#include "LightstreamerRequest.hpp" // Include the header file where LightstreamerRequest is defined.
#include "RequestTutor.hpp" // Include the header file where RequestTutor is defined.
#include "RequestListener.hpp" // Include the header file where RequestListener is defined.

namespace lightstreamer::client::protocol {

    class RequestObjects {
    public:
        const LightstreamerRequest* request; // TODO: Replace with the actual type of LightstreamerRequest
        const RequestTutor* tutor; // TODO: Replace with the actual type of RequestTutor
        const RequestListener* listener; // TODO: Replace with the actual type of RequestListener

        RequestObjects(const LightstreamerRequest* request, const RequestTutor* tutor, const RequestListener* listener)
                : request(request), tutor(tutor), listener(listener) {
        }

        // Depending on your memory management strategy, you might need to define a destructor,
        // copy constructor, and copy assignment operator, especially if you take ownership of the pointers.
        // For this example, the class does not take ownership, assuming these objects are managed elsewhere.

        // Prevent copying to enforce ownership semantics (optional, depending on your use case)
        RequestObjects(const RequestObjects&) = delete;
        RequestObjects& operator=(const RequestObjects&) = delete;

        // Allow move semantics (optional, depending on your use case)
        RequestObjects(RequestObjects&&) noexcept = default;
        RequestObjects& operator=(RequestObjects&&) noexcept = default;

        virtual ~RequestObjects() = default;
    };

} // namespace lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTOBJECTS_HPP
