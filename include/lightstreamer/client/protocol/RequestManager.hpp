/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTMANAGER_HPP

#include <future>
#include <memory>
#include <vector>
#include <lightstreamer/client/protocol/ControlRequestHandler.hpp>
#include <lightstreamer/client/requests/BindSessionRequest.hpp>
#include <lightstreamer/client/transport/RequestHandle.hpp>
#include <lightstreamer/client/protocol/TextProtocol.hpp>

namespace lightstreamer::client::protocol {

    class RequestManager : public ControlRequestHandler {
    public:
        // Destructor should be virtual since this class is intended to be a base class
        ~RequestManager() override = default;

        // bindSession abstract method
        virtual std::unique_ptr<transport::RequestHandle>
        bindSession(std::unique_ptr<requests::BindSessionRequest> request,
                    std::shared_ptr<TextProtocol::StreamListener> reqListener,
                    long tcpConnectTimeout, long tcpReadTimeout, std::future<void> bindFuture) = 0;
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTMANAGER_HPP
