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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_TRANSPORT_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_TRANSPORT_HPP

#include <map>
#include <string>
#include <memory>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/client/transport/RequestListener.hpp>
#include <lightstreamer/client/transport/RequestHandle.hpp>
#include "Protocol.hpp"
#include <lightstreamer/client/Proxy.hpp>

namespace lightstreamer::client::transport {

    class Transport {
    public:
        virtual ~Transport() = default;

        /**
         * Sends a request to the server specified by LightstreamerRequest::getTargetServer().
         * This method must execute quickly. Network/blocking operations must be asynchronously executed on a separate thread.
         *
         * @param protocol The protocol initiating the request.
         * @param request The request to be sent.
         * @param listener The listener that will receive various events for this request.
         * @param extraHeaders HTTP headers to be included in the request to the server (can be empty).
         * @param proxy Coordinates to a proxy to be used to connect to the server (can be nullptr).
         * @param tcpConnectTimeout If the APIs used by the transport allow specifying a connect timeout,
         *                          then use this value. A value of 0 means to keep the underlying transport default.
         * @param tcpReadTimeout If the APIs used by the transport allow specifying a read timeout, then use this value.
         *                       A value of 0 means to keep the underlying transport default.
         * @return A RequestHandle object that allows the caller to notify the Transport that they are no longer
         *         interested in responses to the request. Cannot be nullptr.
         */
        virtual std::unique_ptr<RequestHandle> sendRequest(
                std::shared_ptr<Protocol> protocol,  // TODO: implement Protocol
                std::shared_ptr<LightstreamerRequest> request, // TODO: implement LightstreamerRequest
                std::shared_ptr<RequestListener> listener,
                const std::map<std::string, std::string>& extraHeaders,
                std::shared_ptr<Proxy> proxy,
                long tcpConnectTimeout,
                long tcpReadTimeout) = 0;
    };

} // namespace lightstreamer::client::transport

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TRANSPORT_HPP
