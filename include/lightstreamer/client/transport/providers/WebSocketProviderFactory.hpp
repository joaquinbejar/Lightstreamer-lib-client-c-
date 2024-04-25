/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 25/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPROVIDERFACTORY_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPROVIDERFACTORY_HPP
#include <memory>
#include <lightstreamer/client/session/SessionThread.hpp>
#include <lightstreamer/client/transport/providers/cpp/CppWebSocketProvider.hpp>

namespace lightstreamer::client::transport::providers {
    /**
     * Provides factory methods to create WebSocket providers.
     */
    class WebSocketProviderFactory {
    public:
        /**
         * Creates an instance of NettyWebSocketProvider.
         *
         * @param thread A reference to a SessionThread object.
         * @return A smart pointer to a WebSocketProvider.
         */
        std::unique_ptr<WebSocketProvider> getInstance(std::shared_ptr<session::SessionThread> thread) {
            return std::make_unique<CppWebSocketProvider>();
        }

        /**
         * Checks if the response is buffered.
         *
         * @return false, indicating that the response is not buffered.
         */
        bool isResponseBuffered() const {
            return false;
        }
    };

} // namespace lightstreamer::client::transport::providerscpp

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPROVIDERFACTORY_HPP
