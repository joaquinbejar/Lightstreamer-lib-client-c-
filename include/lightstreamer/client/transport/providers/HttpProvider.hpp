/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 4/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPPROVIDER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPPROVIDER_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/client/protocol/Protocol.hpp>
#include <lightstreamer/client/transport/RequestListener.hpp>
#include <lightstreamer/util/threads/ThreadShutdownHook.hpp>
#include <lightstreamer/client/transport/RequestHandle.hpp>


namespace lightstreamer::client::transport::providers {

    class HttpProvider {
    public:
        virtual ~HttpProvider() = default;

        virtual std::shared_ptr<RequestHandle> createConnection(
                std::shared_ptr<protocol::Protocol> protocol,
                std::shared_ptr<requests::LightstreamerRequest> request,
                std::shared_ptr<RequestListener> httpListener,
                const std::unordered_map<std::string, std::string> &extraHeaders,
                long tcpConnectTimeout,
                long tcpReadTimeout) = 0;

        virtual std::shared_ptr<ThreadShutdownHook> getShutdownHook() const = 0;
    };

    class HttpProvider_HttpRequestListener : public transport::RequestListener {
    };

} // namespace lightstreamer::client::transport::providers

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPPROVIDER_HPP
