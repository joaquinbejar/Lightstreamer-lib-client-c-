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
        void setConnectTimeout(const std::string &value) {
            std::lock_guard<std::mutex> lock(mtx);
            if (value == "AUTO") {
                return; // use default value of retryDelay
            }
            try {
                long n = std::stol(value);
                internal->setRetryDelay(n);
            } catch (const std::invalid_argument &) {
                throw std::invalid_argument(
                        "The given value is not a valid value for setConnectTimeout. Use a positive number or the string \"auto\"");
            }
        }

        /**
         * @brief Gets the current connection timeout.
         *
         * This property is read-only.
         */
        std::string getConnectTimeout() {
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
        std::string getForcedTransport() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getForcedTransport();
        }

        void setForcedTransport(const std::string &value) {
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
        std::map<std::string, std::string> getHttpExtraHeaders() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getHttpExtraHeaders();
        }

        void setHttpExtraHeaders(const std::map<std::string, std::string> &headers) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setHttpExtraHeaders(headers);
        }

        /**
         * @brief Manages the maximum waiting time in milliseconds that the server is allowed to wait for data on a polling connection.
         *
         * This property sets the maximum time the server can wait for data to be sent in response to a polling request,
         * if no data has accumulated at the time of the request. Note that the server might wait longer than this time under certain
         * conditions, such as high load or network constraints.
         *
         * @b Lifecycle: Should be set before calling LightstreamerClient::connect(), but can be modified anytime to affect subsequent polling requests.
         *
         * @b Notifications: Changes to this setting will be communicated to any registered ClientListener through the onPropertyChange event with "idleTimeout".
         *
         * @b Default: 19000 milliseconds (19 seconds).
         */
        long getIdleTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getIdleTimeout();
        }

        void setIdleTimeout(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setIdleTimeout(value);
        }

        /**
         * @brief Sets the interval in milliseconds between two keepalive packets sent on a streaming connection when no data is being transmitted.
         *
         * This property defines the interval at which the server sends keepalive packets to maintain the connection when
         * no data is being transmitted. If set to 0, the server decides the interval based on its configuration.
         *
         * @b Lifecycle: The value should be set before connecting via LightstreamerClient::connect(), but it can be changed anytime.
         * After a connection is established, the server might override this interval based on server-side constraints.
         *
         * @b Notifications: Any change to this setting will be notified through ClientListener::onPropertyChange with "keepaliveInterval".
         *
         * @b Default: 0 (server-configured interval).
         */
        long getKeepaliveInterval() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getKeepaliveInterval();
        }

        void setKeepaliveInterval(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setKeepaliveInterval(value);
        }

        /**
         * @brief Manages the maximum bandwidth requested for the streaming or polling connections, expressed in kbps.
         *
         * The property allows specifying a maximum bandwidth limit for the connection. The special string "unlimited"
         * indicates that no bandwidth limit is imposed by the client, allowing the server to use its default settings.
         * Note that the server or the metadata adapter may enforce a different limit.
         *
         * @note Bandwidth Control is a feature that may be available depending on the edition and license type of the server.
         *
         * @b Lifecycle: Can be set or changed at any time. Changes take effect immediately if a connection is active.
         *
         * @b Notifications: Changes are notified through ClientListener::onPropertyChange with "requestedMaxBandwidth".
         * The server will also notify the client of the actual bandwidth limit enforced, which can be retrieved using
         * RealMaxBandwidth.
         *
         * @b Default: "unlimited"
         */
        std::string getRequestedMaxBandwidth() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getRequestedMaxBandwidth();
        }

        void setRequestedMaxBandwidth(const std::string &value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setRequestedMaxBandwidth(value);
        }

        /**
         * @brief Provides the actual maximum bandwidth that can be consumed, as enforced by the server.
         *
         * This read-only property reports the actual bandwidth used for the connection, which may differ
         * from the requested bandwidth due to server-side restrictions or limitations of the server's bandwidth management.
         * The value is expressed in kbps or as the string "unlimited". If no connection is active, it may return null
         * (or an equivalent unset value).
         *
         * @b Lifecycle: The value is determined when a connection is active and updated based on server communications.
         *
         * @b Notifications: Changes or confirmations are notified through ClientListener::onPropertyChange with "realMaxBandwidth".
         */
        std::string getRealMaxBandwidth() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getRealMaxBandwidth();
        }

        /**
         * @brief Manages the interval in milliseconds between polling requests when in polling mode.
         *
         * This property defines the time between the start of one polling request and the start of the next.
         * Setting it to zero enables "asynchronous polling", where the next polling request is made as soon
         * as the previous one returns. Higher values introduce a delay between polling requests, reducing
         * server load but increasing potential data latency.
         *
         * The server may adjust the interval based on its current load and the client's network conditions.
         * A zero value causes polling requests to be issued back-to-back, which is similar to streaming behavior.
         *
         * @b Lifecycle: Should be set before initiating a connection via LightstreamerClient::connect(),
         * but can be changed at any time to take effect with the next polling request.
         *
         * @b Notifications: Changes are notified through ClientListener::onPropertyChange with "pollingInterval".
         *
         * @b Default: 0 milliseconds, which configures pure "asynchronous polling".
         */
        long getPollingInterval() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getPollingInterval();
        }

        void setPollingInterval(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setPollingInterval(value);
        }

        /**
         * @brief Defines the time in milliseconds the client waits for a keepalive packet or any data after entering "STALLED" status.
         *
         * If the expected keepalive packet or data is not received within this timeout, the client will attempt to reconnect.
         * The reconnection could involve establishing a new session or recovering the existing one, depending on the server's response
         * and session management policies.
         *
         * @b Lifecycle: This value can be adjusted at any time. Changes take effect immediately and apply to the next reconnection attempt.
         *
         * @b Notifications: Changes to this setting are reported to any registered ClientListener through the onPropertyChange event with "reconnectTimeout".
         *
         * @b Default: 3000 milliseconds (3 seconds).
         */
        long getReconnectTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getReconnectTimeout();
        }

        void setReconnectTimeout(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setReconnectTimeout(value);
        }

        /**
         * @brief Controls the delay before attempting a new connection or retry after failure, in milliseconds.
         *
         * This property sets the minimum and maximum delay for:
         * - The minimum time to wait before trying a new connection if the previous one failed.
         * - The maximum time to wait for a response to a request before considering the connection as failed and attempting a different approach.
         *
         * Enforcing a delay between reconnections helps prevent continuous connection attempts when persistent issues are present.
         * This applies to both new session attempts and recovery of current sessions. The delay is calculated from the moment
         * the connection attempt starts, not from when the failure is detected.
         *
         * @note The actual delay may vary if the connection interruption occurs; 'FirstRetryMaxDelay' may apply instead.
         *
         * @b Lifecycle: Can be set and changed at any time.
         *
         * @b Notifications: Changes are reported through ClientListener::onPropertyChange with "retryDelay".
         *
         * @b Default: 4000 milliseconds (4 seconds).
         */
        long getRetryDelay() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getRetryDelay();
        }

        void setRetryDelay(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setRetryDelay(value);
        }

        /**
         * @brief Manages the interval for sending reverse-heartbeats to maintain connection, in milliseconds.
         *
         * Setting this property to a non-zero value enables the reverse-heartbeat mechanism, where the client sends
         * empty control requests at the specified interval to ensure the connection is kept alive. This is particularly useful
         * to prevent communication infrastructure from closing inactive sockets that are ready for reuse, thereby reducing
         * connection reestablishment overhead. It also helps the server detect half-open connections or interruptions in streaming.
         *
         * @b Lifecycle: Should ideally be set before initiating a connection via LightstreamerClient::connect(), but can be adjusted anytime.
         * The new setting takes immediate effect unless a higher heartbeat frequency is already active from the server.
         *
         * @b Notifications: Changes are reported through ClientListener::onPropertyChange with "reverseHeartbeatInterval".
         *
         * @b Default: 0 milliseconds (disabled).
         */
        long getReverseHeartbeatInterval() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getReverseHeartbeatInterval();
        }

        void setReverseHeartbeatInterval(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setReverseHeartbeatInterval(value);
        }

        /**
         * @brief Specifies the extra time in milliseconds the client waits before entering "STALLED" status when a keepalive packet is expected but not received.
         *
         * This timeout determines how long to wait without receiving any data (including keepalive packets) before considering the connection as stalled.
         * It's particularly useful for detecting when a seemingly active connection has effectively gone silent.
         *
         * @b Lifecycle: Can be set and changed at any time.
         *
         * @b Notifications: Changes are reported through ClientListener::onPropertyChange with "stalledTimeout".
         *
         * @b Default: 2000 milliseconds (2 seconds).
         */
        long getStalledTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getStalledTimeout();
        }

        void setStalledTimeout(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setStalledTimeout(value);
        }

        /**
         * @brief Manages the maximum time in milliseconds allowed for attempts to recover the current session after an interruption, beyond which a new session is created.
         *
         * This property sets a limit on how long the client will try to recover a session following an interruption. If recovery is not successful within this timeframe,
         * a new session will be initiated. Setting this to 0 disables any attempt to recover the session, enforcing a fresh connection on interruptions.
         *
         * @note When the server is reached, the recovery process may still fail due to server-side timeouts on session retention. Setting this too low might prevent successful
         * recovery, whereas too high a value may delay the detection of session loss.
         *
         * @b Lifecycle: Can be adjusted at any time.
         *
         * @b Notifications: Changes to this setting are communicated through ClientListener::onPropertyChange with "sessionRecoveryTimeout".
         *
         * @b Default: 15000 milliseconds (15 seconds).
         */
        long getSessionRecoveryTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getSessionRecoveryTimeout();
        }

        void setSessionRecoveryTimeout(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setSessionRecoveryTimeout(value);
        }

        /**
         * @brief Manages the timeout for switch check operations, in milliseconds.
         *
         * This property sets the timeout used to determine how long the client should wait for a response when performing
         * server switch checks during connection or session management operations.
         *
         * @b Lifecycle: Can be set and changed at any time.
         *
         * @b Notifications: Changes to this setting are reported through ClientListener::onPropertyChange with "switchCheckTimeout".
         */
        long getSwitchCheckTimeout() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getSwitchCheckTimeout();
        }

        void setSwitchCheckTimeout(long value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setSwitchCheckTimeout(value);
        }

        /**
         * @brief Enables or disables the early opening of WebSocket connections during session creation.
         *
         * When enabled, a WebSocket connection is opened to the specified server address before receiving
         * any potential specific server instance address during session creation. If a specific server instance
         * address is later received, the initial WebSocket is closed, and a new one is opened to that address.
         *
         * This setting should typically be enabled when the server does not specify a control_link_address
         * and disabled when it does, to ensure connection stability and correct routing of requests within
         * a server cluster.
         *
         * @b Edition Note: The functionality of this setting can depend on server clustering features, which
         * may be available based on the edition and license type of the server infrastructure.
         *
         * @b Lifecycle: Can be adjusted at any time, but changes will only take effect during the next session
         * initiation or request to the server.
         *
         * @b Notifications: Changes are communicated through ClientListener::onPropertyChange with "earlyWSOpenEnabled".
         *
         * @b Default: false
         */
        bool getEarlyWSOpenEnabled() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getEarlyWSOpenEnabled();
        }

        void setEarlyWSOpenEnabled(bool value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setEarlyWSOpenEnabled(value);
        }

        /**
         * @brief Controls whether extra HTTP headers are sent only during session creation.
         *
         * When set to true, extra HTTP headers specified through the HttpExtraHeaders property are only sent during the
         * session creation process. This can be useful to limit header usage to initial handshakes, which might be required
         * for authentication or session initialization. If set to false, the specified headers are sent with every HTTP
         * request or WebSocket establishment.
         *
         * @b Lifecycle: Can be set at any time but is typically configured before initiating a connection through
         * LightstreamerClient::connect(). Changes take effect starting with the next HTTP request or WebSocket connection.
         *
         * @b Notifications: Changes are communicated through ClientListener::onPropertyChange with "httpExtraHeadersOnSessionCreationOnly".
         *
         * @b Default: false (headers are sent with every request).
         */
        bool getHttpExtraHeadersOnSessionCreationOnly() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getHttpExtraHeadersOnSessionCreationOnly();
        }

        void setHttpExtraHeadersOnSessionCreationOnly(bool value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setHttpExtraHeadersOnSessionCreationOnly(value);
        }

        /**
         * @brief Determines whether the client should ignore the server instance address provided during session creation.
         *
         * When enabled, the client ignores any specific server instance addresses provided by the server during session
         * creation and continues to connect using the general server address specified in the connection details. This is
         * useful in environments such as testing where a specific server instance needs to be targeted directly without
         * redirection.
         *
         * @b Lifecycle: Can be adjusted at any time, but changes only apply to subsequent session creation requests.
         *
         * @b Notifications: A change to this setting is notified through ClientListener::onPropertyChange with "serverInstanceAddressIgnored".
         *
         * @b Default: false (the client respects server-provided instance addresses).
         */
        bool getServerInstanceAddressIgnored() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getServerInstanceAddressIgnored();
        }

        void setServerInstanceAddressIgnored(bool value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setServerInstanceAddressIgnored(value);
        }

        /**
         * @brief Enables or disables the slowing algorithm that manages the pace of event processing.
         *
         * This heuristic algorithm detects when the client's CPU is unable to keep up with the rate of events
         * sent by the server on a streaming connection. If enabled and the client is too slow, the client
         * automatically switches to polling mode. In polling mode, the client processes all received data before
         * making the next poll request, which helps manage slow client situations by delaying polls rather than
         * dropping data.
         *
         * @b Lifecycle: Ideally set before connecting through LightstreamerClient::connect(), but can be changed
         * at any time to affect subsequent streaming connections.
         *
         * @b Notifications: Changes are reported through ClientListener::onPropertyChange with "slowingEnabled".
         *
         * @b Default: false (disabled, streaming operates at normal pace without automatic adjustment).
         */
        bool getSlowingEnabled() {
            std::lock_guard<std::mutex> lock(mtx);
            return internal->getSlowingEnabled();
        }

        void setSlowingEnabled(bool value) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setSlowingEnabled(value);
        }

        /**
         * @brief Configures the proxy server settings to be used for connections to the Lightstreamer Server.
         *
         * This setter method allows specifying the details of a proxy server through which all connections
         * to the Lightstreamer Server should be routed. Setting this to a non-null value enables proxy usage,
         * while setting it to null (or an equivalent unset state) disables proxy routing.
         *
         * @b Lifecycle: Can be set and changed at any time. The new settings will be used starting from the next connection attempt.
         *
         * @b Notifications: Any change to this setting will be notified through ClientListener::onPropertyChange with "proxy".
         *
         * @b Default: nullptr (no proxy used).
         */
        void setProxy(const std::shared_ptr<Proxy> &proxy) {
            std::lock_guard<std::mutex> lock(mtx);
            internal->setProxy(proxy);
        }

    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONNECTIONOPTIONS_HPP
