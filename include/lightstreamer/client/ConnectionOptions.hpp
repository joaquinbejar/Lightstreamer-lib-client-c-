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

        /**
         * @brief Manages the maximum waiting time in milliseconds before attempting a new connection to the server.
         *
         * If the previous connection is unexpectedly closed while correctly working, this property determines the maximum
         * delay before trying a new connection, which may be a new session or a recovery of the current session. The actual
         * delay applied is a random value between 0 and this maximum value to help avoid load spikes on the server cluster
         * due to simultaneous reconnections.
         *
         * @note This delay is only applied before the first reconnection. If this reconnection fails, the RetryDelay setting
         * is applied.
         *
         * @b Lifecycle: This value can be set and changed at any time.
         *
         * @b Related notifications: Changes to this setting are notified through a call to ClientListener::onPropertyChange
         * with argument "firstRetryMaxDelay" on any ClientListener listening to the related LightstreamerClient.
         *
         * @b Default value: 100 milliseconds (0.1 seconds).
         */
        long getFirstRetryMaxDelay() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getFirstRetryMaxDelay();
        }

        void setFirstRetryMaxDelay(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setFirstRetryMaxDelay(value);
        }

        /**
         * @brief Manages the timeout for forcing a binding to the server.
         *
         * This property specifies how long the client will wait before timing out a binding operation to the server.
         *
         * @b Lifecycle: This value can be set and changed at any time.
         *
         * @b Related notifications: Changes to this setting are notified through a call to ClientListener::onPropertyChange
         * with argument "forceBindTimeout" on any ClientListener listening to the related LightstreamerClient.
         */
        long getForceBindTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getForceBindTimeout();
        }

        void setForceBindTimeout(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setForceBindTimeout(value);
        }

        /**
         * @brief Allows enabling or disabling the Stream-Sense algorithm and forcing a specific transport or transport/connection type.
         *
         * This property can control the connection behavior, enabling or disabling Stream-Sense and forcing the use of a specific
         * transport or combination of transport and connection type. Specifying a combination disables Stream-Sense completely.
         *
         * @note If the Stream-Sense algorithm is disabled, the client may still enter "CONNECTED:STREAM-SENSING" status.
         * If streaming is found to be impossible, no recovery attempt will be made.
         *
         * Possible values:
         * - nullptr: Stream-Sense is enabled, and the client connects using the most appropriate transport and connection type.
         * - "WS": WebSocket only. If WebSocket connections cannot be established, no connection will be made.
         * - "HTTP": HTTP only. If HTTP connections cannot be established, no connection will be made.
         * - "WS-STREAMING": Only WebSocket streaming. No connection if this is not possible.
         * - "HTTP-STREAMING": Only HTTP streaming. No connection if this is not possible.
         * - "WS-POLLING": Only WebSocket polling. No connection if this is not possible.
         * - "HTTP-POLLING": Only HTTP polling. No connection if this is not possible.
         *
         * @b Lifecycle: Can be set at any time. If set during a connection, it will trigger a switch to the specified configuration.
         *
         * @b Notifications: Changes are notified through ClientListener::onPropertyChange with "forcedTransport".
         *
         * @b Default: nullptr (Stream-Sense enabled).
         */
        std::string getForcedTransport()  {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getForcedTransport();
        }

        void setForcedTransport(const std::string& value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setForcedTransport(value);
        }

        /**
       * @brief Manages extra HTTP headers to be sent with requests to the server.
       *
       * This property allows setting extra HTTP headers that will be included in all requests to the server.
       * The Content-Type header is reserved and managed by the client library itself. Usage of certain headers
       * might be restricted based on the environment, and some may even prevent successful connection establishment.
       *
       * @note Custom cookies should be managed through LightstreamerClient::addCookies instead of this property.
       * Setting headers might trigger an OPTIONS request before establishing the actual connection.
       *
       * @b Lifecycle: Should be set before LightstreamerClient::connect() but can be changed anytime for subsequent requests.
       *
       * @b Notifications: Changes are notified through ClientListener::onPropertyChange with "httpExtraHeaders".
       *
       * @b Default: nullptr (no extra headers sent).
       *
       * @see HttpExtraHeadersOnSessionCreationOnly
       */
        std::map<std::string, std::string> getHttpExtraHeaders()  {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getHttpExtraHeaders();
        }

        void setHttpExtraHeaders(const std::map<std::string, std::string>& headers) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setHttpExtraHeaders(headers);
        }


    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONNECTIONOPTIONS_HPP
