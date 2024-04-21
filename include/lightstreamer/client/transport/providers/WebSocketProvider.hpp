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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPROVIDER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPROVIDER_HPP

#include <memory>
#include <string>
#include <unordered_map>

namespace lightstreamer::client::transport::providers {
    // Forward declarations for dependencies
    class SessionRequestListener;

    class RequestListener;

    class ThreadShutdownHook;

    class Proxy; // Assuming Proxy is a previously defined class

    /**
     * Interface used to decouple the application classes from a specific WebSocket implementation.
     * Instances of this type are obtained through the factory TransportFactory::getDefaultWebSocketFactory().
     */
    class WebSocketProvider {
    public:
        /**
         * Opens a WebSocket connection.
         *
         * @param address Host address.
         * @param networkListener Listens to connection events (opening, closing, message receiving, error).
         * @param extraHeaders Headers to be added during WebSocket handshake.
         * @param cookies Cookies to be added during WebSocket handshake.
         * @param proxy If not null, the client connects to the proxy and the proxy forwards the messages to the host.
         * @param timeout Connection timeout in milliseconds.
         */
        virtual void connect(const std::string &address,
                             std::shared_ptr<SessionRequestListener> networkListener,
                             const std::unordered_map<std::string, std::string> &extraHeaders,
                             const std::string &cookies, std::shared_ptr<Proxy> proxy, long timeout) = 0;

        /**
         * Sends a message.
         *
         * Note: When the message has been successfully written on WebSocket,
         * it is mandatory to notify the method RequestListener::onOpen().
         *
         * @param message The message to be sent.
         * @param listener Listener to notify about the request's outcome.
         */
        virtual void send(const std::string &message, std::shared_ptr<RequestListener> listener) = 0;

        /**
         * Closes the connection.
         */
        virtual void disconnect() = 0;

        /**
         * Returns a callback to free the resources (threads, sockets...) allocated by the provider.
         *
         * @return A smart pointer to the ThreadShutdownHook.
         */
        virtual std::shared_ptr<ThreadShutdownHook> getThreadShutdownHook() const = 0;

        // Ensure virtual destructor for proper cleanup of derived classes
        virtual ~WebSocketProvider() = default;
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPROVIDER_HPP
