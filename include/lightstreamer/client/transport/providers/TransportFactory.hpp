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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_TRANSPORTFACTORY_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_TRANSPORTFACTORY_HPP

#include <memory>
#include <stdexcept>
#include <mutex>
#include <iostream>
#include <lightstreamer/client/session/SessionThread.hpp>
#include <lightstreamer/util/threads/ThreadShutdownHook.hpp>
#include <lightstreamer/client/transport/providers/HttpProvider.hpp>
#include <lightstreamer/client/transport/providers/WebSocketProvider.hpp>


namespace lightstreamer::client::transport::providers {


    /**
     * A transport factory creates instances of a specific transport implementation.
     *
     * Note:
     * An abstract class is used instead of an interface because static methods are needed
     * for returning the default factory implementations.
     */
    template<typename T>
    class TransportFactory {
    public:
        /**
         * Returns a new instance of a transport.
         *
         * @param thread A reference to a SessionThread object.
         * @return A smart pointer to a transport instance.
         */
        virtual std::unique_ptr<T> getInstance(std::shared_ptr<session::SessionThread> thread) = 0;

        /**
         * Returns true if the transport implementation reads the whole response before passing it to the client.
         * When the response is buffered, the content-length should be small (about 4Mb).
         *
         * @return A boolean indicating if the response is buffered.
         */
        virtual bool isResponseBuffered() const = 0;

    private:
        static std::unique_ptr<TransportFactory<HttpProvider>> defaultHttpFactory;
        static std::unique_ptr<TransportFactory<WebSocketProvider>> defaultWSFactory;
        static std::shared_ptr<ThreadShutdownHook> transportShutdownHook;

        // Synchronization mutex
        static std::mutex mutex;

    public:
        /**
         * Returns the default HTTP factory.
         *
         * @return A smart pointer to the default HTTP transport factory.
         */
        static std::unique_ptr<TransportFactory<HttpProvider>>& getDefaultHttpFactory() {
            std::lock_guard<std::mutex> lock(mutex);
            if (!defaultHttpFactory) {
                // Initialize with default or alternative HTTP factory
                // defaultHttpFactory = std::make_unique<NettyHttpProviderFactory>();
                // Placeholder for actual initialization
            }
            return defaultHttpFactory;
        }

        /**
         * Sets the default HTTP factory.
         *
         * @param factory A smart pointer to an HTTP transport factory.
         */
        static void setDefaultHttpFactory(std::unique_ptr<TransportFactory<HttpProvider>> factory) {
            std::lock_guard<std::mutex> lock(mutex);
            if (!factory) {
                throw std::invalid_argument("Specify a factory");
            }
            defaultHttpFactory = std::move(factory);
        }

        /**
         * Returns the default WebSocket factory.
         *
         * @return A smart pointer to the default WebSocket transport factory.
         */
        static std::unique_ptr<TransportFactory<WebSocketProvider>>& getDefaultWebSocketFactory() {
            std::lock_guard<std::mutex> lock(mutex);
            if (!defaultWSFactory) {
                // Initialize with default or alternative WebSocket factory
                // defaultWSFactory = std::make_unique<WebSocketProviderFactory>();
                // Placeholder for actual initialization
            }
            return defaultWSFactory;
        }

        /**
         * Sets the default WebSocket factory.
         *
         * @param factory A smart pointer to a WebSocket transport factory.
         */
        static void setDefaultWebSocketFactory(std::unique_ptr<TransportFactory<WebSocketProvider>> factory) {
            std::lock_guard<std::mutex> lock(mutex);
            defaultWSFactory = std::move(factory);
        }

        /**
         * Returns the shutdown hook releasing the resources shared by the transport providers (e.g., socket pools).
         *
         * @return A shared pointer to the transport shutdown hook.
         */
        static std::shared_ptr<ThreadShutdownHook> getTransportShutdownHook() {
            std::lock_guard<std::mutex> lock(mutex);
            if (!transportShutdownHook) {
                // Initialize transport shutdown hook
                // Placeholder for actual initialization
            }
            return transportShutdownHook;
        }

        /**
         * Sets the transport shutdown hook.
         *
         * @param hook A shared pointer to a ThreadShutdownHook.
         */
        static void setTransportShutdownHook(std::shared_ptr<ThreadShutdownHook> hook) {
            std::lock_guard<std::mutex> lock(mutex);
            transportShutdownHook = hook;
        }
    };

    // Static member definitions
    template<typename T>
    std::unique_ptr<TransportFactory<HttpProvider>> TransportFactory<T>::defaultHttpFactory = nullptr;

    template<typename T>
    std::unique_ptr<TransportFactory<WebSocketProvider>> TransportFactory<T>::defaultWSFactory = nullptr;

    template<typename T>
    std::shared_ptr<ThreadShutdownHook> TransportFactory<T>::transportShutdownHook = nullptr;

    template<typename T>
    std::mutex TransportFactory<T>::mutex;
}


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_TRANSPORTFACTORY_HPP
