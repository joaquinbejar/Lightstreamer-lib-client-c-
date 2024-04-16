/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CONNECTIONOPTIONS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CONNECTIONOPTIONS_HPP
#include <memory>
#include <string>
#include <map>
#include <stdexcept>
#include <mutex>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>

namespace lightstreamer::client {

    /**
     * @brief Represents a class used by LightstreamerClient to provide extra connection properties.
     *
     * This class acts as a proxy for the InternalConnectionOptions class to expose certain elements.
     * An instance of this class is attached to every LightstreamerClient as LightstreamerClient.connectionOptions.
     */
    class ConnectionOptions {
        std::unique_ptr<session::InternalConnectionOptions> internal;
        std::mutex mtx;

    public:
        explicit ConnectionOptions(std::unique_ptr<session::InternalConnectionOptions> internal)
                : internal(std::move(internal)) {}

        /**
         * @brief Gets or sets the ConnectTimeout property.
         *
         * ConnectTimeout is deprecated, please use RetryDelay instead.
         *
         * @throws std::invalid_argument If the provided value is not valid.
         */
        void setConnectTimeout(const std::string& value) {
            std::lock_guard<std::mutex> lock(mtx);
            if (value == "AUTO") {
                return; // use default value of retryDelay
            }
            try {
                long n = std::stol(value);
                internal->setRetryDelay(n);
            } catch (const std::invalid_argument&) {
                throw std::invalid_argument("The given value is not a valid value for setConnectTimeout. Use a positive number or the string \"auto\"");
            }
        }

        /**
         * @brief Gets the current connection timeout.
         *
         * This property is read-only.
         */
        std::string getConnectTimeout()  {
            std::lock_guard<std::mutex> lock(mtx);
            return std::to_string(internal->getRetryDelay());
        }

        /**
         * @brief Gets the current connection timeout.
         *
         * This is the extra time that the system may wait for a response before deciding to drop the
         * connection and try another approach. This timeout is managed internally and may vary based
         * on network conditions.
         *
         * @return The current connect timeout in milliseconds.
         */
        long getCurrentConnectTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getCurrentConnectTimeout();
        }

        /**
         * @brief Manages the length in bytes used by the server for the response body on a stream connection.
         *
         * This property specifies the length that the server will use for the response body in an HTTP-STREAMING connection.
         * Once the specified content length is exhausted, the connection will be closed and a new bind connection
         * will be automatically reopened. This setting is not applicable to WebSocket connections.
         *
         * @note This setting only applies to the "HTTP-STREAMING" case, not to WebSockets.
         *
         * @b Lifecycle: The content length should be set before calling LightstreamerClient::connect().
         * However, the value can be changed at any time, and the new value will be used for the next streaming
         * connection (either a bind or a brand new session).
         *
         * @b Related notifications: A change to this setting will be notified through a call to
         * ClientListener::onPropertyChange with argument "contentLength" on any ClientListener listening to
         * the related LightstreamerClient.
         *
         * @b Default value: The default length is decided by the library to ensure the best performance,
         * which may be a few MB or much higher, depending on the environment.
         */
        /**
         * @brief Gets the content length used for HTTP streaming connections.
         *
         * @return The content length in bytes.
         */
        long getContentLength() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getContentLength();
        }

        /**
         * @brief Sets the content length used for HTTP streaming connections.
         *
         * @param value The new content length in bytes.
         */
        void setContentLength(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setContentLength(value);
        }





    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONNECTIONOPTIONS_HPP
