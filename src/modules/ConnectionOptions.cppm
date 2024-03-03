/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 2/3/24
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

export module ConnectionOptions;

import <string>;

namespace Lightstreamer::Cpp::ConnectionOptions {

    typedef int ContentLength; // TODO: Implement this type
    typedef int FirstRetryMaxDelay; // TODO: Implement this type
    typedef int ForcedTransport; // TODO: Implement this type
    typedef int HttpExtraHeaders; // TODO: Implement this type
    typedef int HttpExtraHeadersOnSessionCreationOnly; // TODO: Implement this type
    typedef int IdleTimeout; // TODO: Implement this type
    typedef int KeepaliveInterval; // TODO: Implement this type
    typedef int PollingInterval; // TODO: Implement this type
    typedef int Proxy; // TODO: Implement this type
    typedef int ReconnectTimeout; // TODO: Implement this type
    typedef int MaxBandwidth; // TODO: Implement this type
    typedef int RetryDelay; // TODO: Implement this type
    typedef int ReverseHeartbeatInterval; // TODO: Implement this type
    typedef int ServerInstanceAddressIgnored; // TODO: Implement this type
    typedef int SessionRecoveryTimeout; // TODO: Implement this type
    typedef int SlowingEnabled; // TODO: Implement this type
    typedef int StalledTimeout; // TODO: Implement this type


    /**
    Used by LightstreamerClient to provide an extra connection properties data object.

    Data object that contains the policy settings used to connect to a Lightstreamer Server.

    An instance of this class is attached to every \ref `LightstreamerClient`
    as \ref `LightstreamerClient.connectionOptions`

    .. seealso:: \ref `LightstreamerClient`
    **/
    class ConnectionOptions {

        /**
        Inquiry method that gets the length expressed in bytes to be used by the Server for the response body on a 
        HTTP stream connection.

        @return: The length to be used by the Server for the response body on a HTTP stream connection

        .. seealso:: \ref `setContentLength`
        **/
        void getContentLength() {}

        /**
        Inquiry method that gets the maximum time to wait before trying a new connection to the Server in case the 
        previous one is unexpectedly closed while correctly working.

        @return: The max time (in milliseconds) to wait before trying a new connection.

        .. seealso:: \ref `setFirstRetryMaxDelay`
        **/
        void getFirstRetryMaxDelay() {}

        /**
        Inquiry method that gets the value of the forced transport (if any).

        @return: The forced transport or None

        .. seealso:: \ref `setForcedTransport`
        **/
        void getForcedTransport() {}

        /**
        Inquiry method that gets the Map object containing the extra headers to be sent to the server.

        @return: The Map object containing the extra headers to be sent

        .. seealso:: \ref `setHttpExtraHeaders`
        .. seealso:: \ref `setHttpExtraHeadersOnSessionCreationOnly`
        **/
        void getHttpExtraHeaders() {}

        /**
        Inquiry method that gets the maximum time the Server is allowed to wait for any data to be sent
        in response to a polling request, if none has accumulated at request time. The wait time used
        by the Server, however, may be different, because of server side restrictions.

        @return: The time (in milliseconds) the Server is allowed to wait for data to send upon polling requests.

        .. seealso:: \ref `setIdleTimeout`
        **/
        void getIdleTimeout() {}

        /**
        Inquiry method that gets the interval between two keepalive packets sent by Lightstreamer Server
        on a stream connection when no actual data is being transmitted. If the returned value is 0,
        it means that the interval is to be decided by the Server upon the next connection.

        @b lifecycle If the value has just been set and a connection to Lightstreamer Server has not been
                established yet, the returned value is the time that is being requested to the Server.
        Afterwards, the returned value is the time used by the Server, that may be different, because
                of Server side constraints.

        @return: The time, expressed in milliseconds, between two keepalive packets sent by the Server, or 0.

        .. seealso:: \ref `setKeepaliveInterval`
        **/
        void getKeepaliveInterval() {}

        /**
        Inquiry method that gets the polling interval used for polling connections.

        If the value has just been set and a polling request to Lightstreamer Server has not been performed
        yet, the returned value is the polling interval that is being requested to the Server. Afterwards,
        the returned value is the the time between subsequent polling requests that is really allowed by the
                Server, that may be different, because of Server side constraints.

        @return: The time (in milliseconds) between subsequent polling requests.

        .. seealso:: \ref `setPollingInterval`
        **/
        void getPollingInterval() {}

        /**
        Inquiry method that gets the maximum bandwidth that can be consumed for the data coming from
        Lightstreamer Server. This is the actual maximum bandwidth, in contrast with the requested
        maximum bandwidth, returned by \ref `getRequestedMaxBandwidth`.

        The value may differ from the requested one because of restrictions operated on the server side,
        or because bandwidth management is not supported (in this case it is always "unlimited"),
        but also because of number rounding.

        @b lifecycle If a connection to Lightstreamer Server is not currently active, null is returned; soon after the connection is established, the value will become available.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "realMaxBandwidth" on any
        ClientListener listening to the related LightstreamerClient.

        @return:  A decimal number, which represents the maximum bandwidth applied by the Server for the streaming or polling connection expressed in kbps (kilobits/sec), or the string "unlimited", or None.

        .. seealso:: \ref `setRequestedMaxBandwidth`
        **/
        void getRealMaxBandwidth() {}

        /**
        Inquiry method that gets the time the client, after entering "STALLED" status,
        is allowed to keep waiting for a keepalive packet or any data on a stream connection,
        before disconnecting and trying to reconnect to the Server.

        @return: The idle time (in milliseconds) admitted in "STALLED" status before trying to reconnect to the Server.

        .. seealso:: \ref `setReconnectTimeout`
        **/
        void getReconnectTimeout() {}

        /**
        Inquiry method that gets the maximum bandwidth that can be consumed for the data coming from
        Lightstreamer Server, as requested for this session.
        The maximum bandwidth limit really applied by the Server on the session is provided by
                \ref `getRealMaxBandwidth`

        @return:  A decimal number, which represents the maximum bandwidth requested for the streaming or polling 
         connection expressed in kbps (kilobits/sec), or the string "unlimited".

        .. seealso:: \ref `setRequestedMaxBandwidth`
        **/
        void getRequestedMaxBandwidth() {}

        /**
		Inquiry method that gets the minimum time to wait before trying a new connection
        to the Server in case the previous one failed for any reason, which is also the maximum time to wait for a response to a request
                before dropping the connection and trying with a different approach.
        Note that the delay is calculated from the moment the effort to create a connection
                is made, not from the moment the failure is detected or the connection timeout expires.

        @return: The time (in milliseconds) to wait before trying a new connection.

        .. seealso:: \ref `setRetryDelay`
        **/
        void getRetryDelay() {}

        /**
		Inquiry method that gets the reverse-heartbeat interval expressed in milliseconds.
        A 0 value is possible, meaning that the mechanism is disabled.

        @return: The reverse-heartbeat interval, or 0.

        .. seealso:: \ref `setReverseHeartbeatInterval`
        **/
        void getReverseHeartbeatInterval() {}

        /**
		Inquiry method that gets the maximum time allowed for attempts to recover
        the current session upon an interruption, after which a new session will be created.
        A 0 value also means that any attempt to recover the current session is prevented
                in the first place.

        @return: The maximum time allowed for recovery attempts, possibly 0.

        .. seealso:: \ref `setSessionRecoveryTimeout`
        **/
        void getSessionRecoveryTimeout() {}

        /**
	    Inquiry method that gets the extra time the client can wait when an expected keepalive packet
        has not been received on a stream connection (and no actual data has arrived), before entering
        the "STALLED" status.

        @return: The idle time (in milliseconds) admitted before entering the "STALLED" status.

        .. seealso:: \ref `setStalledTimeout`
        **/
        void getStalledTimeout() {}

        /**
		Inquiry method that checks if the restriction on the forwarding of the configured extra http headers
        applies or not.

        @return: true/false if the restriction applies or not.

        .. seealso:: \ref `setHttpExtraHeadersOnSessionCreationOnly`
        .. seealso:: \ref `setHttpExtraHeaders`
        **/
        void isHttpExtraHeadersOnSessionCreationOnly() {}

        /**
		Inquiry method that checks if the client is going to ignore the server instance address that
        will possibly be sent by the server.

        @return: Whether or not to ignore the server instance address sent by the server.

        .. seealso:: \ref `setServerInstanceAddressIgnored`
        **/
        void isServerInstanceAddressIgnored() {}

        /**
	    Inquiry method that checks if the slowing algorithm is enabled or not.

        @return: Whether the slowing algorithm is enabled or not.

        .. seealso:: \ref `setSlowingEnabled`
        **/
        void isSlowingEnabled() {}

        /**
		Setter method that sets the length in bytes to be used by the Server for the response body on a stream connection
        (a minimum length, however, is ensured by the server). After the content length exhaustion, the connection will
        be closed and a new bind connection will be automatically reopened.

        NOTE that this setting only applies to the "HTTP-STREAMING" case (i.e. not to WebSockets).

        @b default A length decided by the library, to ensure the best performance.
        It can be of a few MB or much higher, depending on the environment.

        @b lifecycle The content length should be set before calling
        the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the supplied value will
        be used for the next streaming connection (either a bind or a brand new session).

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "contentLength" on any
        ClientListener listening to the related LightstreamerClient.

        @param contentLength: The length to be used by the Server for the response body on a HTTP stream connection.

        @throws IllegalArgumentException: if a negative or zero value is configured
        **/
        void setContentLength(ContentLength contentlength) {}

        /**
		Setter method that sets the maximum time to wait before trying a new connection to the Server
        in case the previous one is unexpectedly closed while correctly working.
        The new connection may be either the opening of a new session or an attempt to recovery
        the current session, depending on the kind of interruption.

        The actual delay is a randomized value between 0 and this value.
        This randomization might help avoid a load spike on the cluster due to simultaneous reconnections, should one of
                the active servers be stopped. Note that this delay is only applied before the first reconnection: should such
        reconnection fail, only the setting of \ref `setRetryDelay` will be applied.

        @b default 100 (0.1 seconds)

        @b lifecycle This value can be set and changed at any time.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "firstRetryMaxDelay" on any
        ClientListener listening to the related LightstreamerClient.

        @param firstRetryMaxDelay: The max time (in milliseconds) to wait before trying a new connection.

        @throws IllegalArgumentException: if a negative or zero value is configured
        **/
        void setFirstRetryMaxDelay(FirstRetryMaxDelay firstretrymaxdelay) {}

        /**
		Setter method that can be used to disable/enable the Stream-Sense algorithm and to force the client to use a fixed
        transport or a fixed combination of a transport and a connection type. When a combination is specified the
        Stream-Sense algorithm is completely disabled.

        The method can be used to switch between streaming and polling connection types and between
        HTTP and WebSocket transports.

        In some cases, the requested status may not be reached, because of connection or environment problems. In that case
        the client will continuously attempt to reach the configured status.

        Note that if the Stream-Sense algorithm is disabled, the client may still enter the "CONNECTED:STREAM-SENSING" status;
        however, in that case, if it eventually finds out that streaming is not possible, no recovery will be tried.

        @b default None (full Stream-Sense enabled).

        @b lifecycle This method can be called at any time. If called while the client is connecting or connected it will instruct
        to switch connection type to match the given configuration.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "forcedTransport" on any
        ClientListener listening to the related LightstreamerClient.

        @param forcedTransport: can be one of the following:

        * None: the Stream-Sense algorithm is enabled and the client will automatically connect using the most appropriate transport and connection type among those made possible by the environment.
        * "WS": the Stream-Sense algorithm is enabled as in the None case but the client will only use WebSocket based connections. If a connection over WebSocket is not possible because of the environment the client will not connect at all.
        * "HTTP": the Stream-Sense algorithm is enabled as in the None case but the client will only use HTTP based connections. If a connection over HTTP is not possible because of the environment the client will not connect at all.
        * "WS-STREAMING": the Stream-Sense algorithm is disabled and the client will only connect on Streaming over WebSocket. If Streaming over WebSocket is not possible because of the environment the client will not connect at all.
        * "HTTP-STREAMING": the Stream-Sense algorithm is disabled and the client will only connect on Streaming over HTTP. If Streaming over HTTP is not possible because of the browser/environment the client will not connect at all.
        * "WS-POLLING": the Stream-Sense algorithm is disabled and the client will only connect on Polling over WebSocket. If Polling over WebSocket is not possible because of the environment the client will not connect at all.
        * "HTTP-POLLING": the Stream-Sense algorithm is disabled and the client will only connect on Polling over HTTP. If Polling over HTTP is not possible because of the environment the client will not connect at all.

        @throws IllegalArgumentException: if the given value is not in the list of the admitted ones.
        **/
        void setForcedTransport(ForcedTransport forcedtransport) {}

        /**
		Setter method that enables/disables the setting of extra HTTP headers to all the request performed to the
        Lightstreamer server by the client.

        Note that the Content-Type header is reserved by the client library itself, while other headers might be
        refused by the environment and others might cause the connection to the server to fail.

        For instance, you cannot use this method to specify custom cookies to be sent to Lightstreamer Server; leverage
         \ref `LightstreamerClient.addCookies` instead.
        The use of custom headers might also cause the client to send an OPTIONS request to the server before
        opening the actual connection.

        @b default None (meaning no extra headers are sent).

        @b lifecycle This setting should be performed before calling the
        \ref `LightstreamerClient.connect` method. However, the value can be changed
        at any time: the supplied value will be used for the next HTTP request or WebSocket establishment.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "httpExtraHeaders" on any
        ClientListener listening to the related LightstreamerClient.

        @param httpExtraHeaders: a Map object containing header-name header-value pairs. None can be specified to avoid
         extra headers to be sent.
        **/
        void setHttpExtraHeaders(HttpExtraHeaders httpextraheaders) {}

        /**
		Setter method that enables/disables a restriction on the forwarding of the extra http headers specified through
        \ref `setHttpExtraHeaders`. If true, said headers will only be sent during the session creation
        process (and thus will still be available to the metadata adapter notifyUser method) but will not be sent on following
        requests. On the contrary, when set to true, the specified extra headers will be sent to the server on every request.

        @b default false

        @b lifecycle This setting should be performed before calling the
                \ref `LightstreamerClient.connect` method. However, the value can be changed
                at any time: the supplied value will be used for the next HTTP request or WebSocket establishment.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "httpExtraHeadersOnSessionCreationOnly" on any
        ClientListener listening to the related LightstreamerClient.

        @param httpExtraHeadersOnSessionCreationOnly: true/false to enable/disable the restriction on extra headers forwarding.
        **/
        void setHttpExtraHeadersOnSessionCreationOnly(
                HttpExtraHeadersOnSessionCreationOnly httpextraheadersonsessioncreationonly) {}

        /**
		Setter method that sets the maximum time the Server is allowed to wait for any data to be sent in response to a
        polling request, if none has accumulated at request time. Setting this time to a nonzero value and the polling interval
        to zero leads to an "asynchronous polling" behavior, which, on low data rates, is very similar to the streaming case.
        Setting this time to zero and the polling interval to a nonzero value, on the other hand, leads to a classical
        "synchronous polling".

        Note that the Server may, in some cases, delay the answer for more than the supplied time, to protect itself against
        a high polling rate or because of bandwidth restrictions. Also, the Server may impose an upper limit on the wait time,
        in order to be able to check for client-side connection drops.

        @b default  19000 (19 seconds).

        @b lifecycle The idle timeout should be set before calling the
                \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the supplied value
                will be used for the next polling request.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "idleTimeout" on any
        ClientListener listening to the related LightstreamerClient.

        @param idleTimeout: The time (in milliseconds) the Server is allowed to wait for data to send upon polling requests.

        @throws IllegalArgumentException: if a negative value is configured
        **/
        void setIdleTimeout(IdleTimeout idletimeout) {}

        /**
		Setter method that sets the interval between two keepalive packets to be sent by Lightstreamer Server on a stream
        connection when no actual data is being transmitted. The Server may, however, impose a lower limit on the keepalive
        interval, in order to protect itself. Also, the Server may impose an upper limit on the keepalive interval, in
        order to be able to check for client-side connection drops.
        If 0 is specified, the interval will be decided by the Server.

        @b default 0 (meaning that the Server will send keepalive packets based on its own configuration).

        @b lifecycle The keepalive interval should be set before
        calling the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the supplied
        value will be used for the next streaming connection (either a bind or a brand new session). Note that, after a connection, the value may be changed to the one imposed by the Server.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "keepaliveInterval" on any
        ClientListener listening to the related LightstreamerClient.

        @param keepaliveInterval: the keepalive interval time (in milliseconds) to set, or 0.

        @throws IllegalArgumentException: if a negative value is configured

        .. seealso:: \ref `setStalledTimeout`
        .. seealso:: \ref `setReconnectTimeout`
        **/
        void setKeepaliveInterval(KeepaliveInterval keepaliveinterval) {}

        /**
		Setter method that sets the polling interval used for polling connections. The client switches from the default
        streaming mode to polling mode when the client network infrastructure does not allow streaming. Also,
        polling mode can be forced by calling \ref `setForcedTransport` with "WS-POLLING" or "HTTP-POLLING"
        as parameter.

        The polling interval affects the rate at which polling requests are issued. It is the time between the start of a
        polling request and the start of the next request. However, if the polling interval expires before the first polling
        request has returned, then the second polling request is delayed. This may happen, for instance, when the Server delays
        the answer because of the idle timeout setting. In any case, the polling interval allows for setting an upper limit on
        the polling frequency.

        The Server does not impose a lower limit on the client polling interval. However, in some cases, it may protect itself
        against a high polling rate by delaying its answer. Network limitations and configured bandwidth limits may also lower
        the polling rate, despite of the client polling interval.

        The Server may, however, impose an upper limit on the polling interval, in order to be able to promptly detect
                terminated polling request sequences and discard related session information.

        @b default 0 (pure "asynchronous polling" is configured).

        @b lifecycle The polling interval should be set before calling
                the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the supplied value will
        be used for the next polling request.

        Note that, after each polling request, the value may be changed to the one imposed by the Server.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "pollingInterval" on any
        ClientListener listening to the related LightstreamerClient.

        @param pollingInterval: The time (in milliseconds) between subsequent polling requests. Zero is a legal value too, meaning that the client will issue a new polling request as soon as a previous one has returned.

        @throws IllegalArgumentException: if a negative value is configured
        **/
        void setPollingInterval(PollingInterval pollinginterval) {}

        /**
		Setter method that configures the coordinates to a proxy server to be used to connect to the Lightstreamer Server.

        @b default None (meaning not to pass through a proxy).

        @b lifecycle This value can be set and changed at any time. The supplied value will
        be used for the next connection attempt.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "proxy" on any
        ClientListener listening to the related LightstreamerClient.

        @param proxy: The proxy configuration. Specify None to avoid using a proxy.
        **/
        void setProxy(Proxy proxy) {}

        /**
		Setter method that sets the time the client, after entering "STALLED" status,
        is allowed to keep waiting for a keepalive packet or any data on a stream connection,
                before disconnecting and trying to reconnect to the Server.
        The new connection may be either the opening of a new session or an attempt to recovery
        the current session, depending on the kind of interruption.

        @b default 3000 (3 seconds).

        @b lifecycle This value can be set and changed at any time.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "reconnectTimeout" on any
        ClientListener listening to the related LightstreamerClient.

        @param reconnectTimeout: The idle time (in milliseconds) allowed in "STALLED" status before trying to reconnect to the Server.

        @throws IllegalArgumentException: if a negative or zero value is configured

        .. seealso:: \ref `setStalledTimeout`
        .. seealso:: \ref `setKeepaliveInterval`
        **/
        void setReconnectTimeout(ReconnectTimeout reconnecttimeout) {}

        /**
		Setter method that sets the maximum bandwidth expressed in kilobits/s that can be consumed for the data coming from
        Lightstreamer Server. A limit on bandwidth may already be posed by the Metadata Adapter, but the client can
                furtherly restrict this limit. The limit applies to the bytes received in each streaming or polling connection.

        **general edition note** Bandwidth Control is an optional feature, available depending on Edition and License Type.
        To know what features are enabled by your license, please see the License tab of the Monitoring Dashboard (by default,
        available at /dashboard).

        @b default "unlimited"

        @b lifecycle The bandwidth limit can be set and changed at any time. If a connection is currently active, the bandwidth
                limit for the connection is changed on the fly. Remember that the Server may apply a different limit.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "requestedMaxBandwidth" on any
        ClientListener listening to the related LightstreamerClient. Moreover, upon any change or attempt to change the
        limit, the Server will notify the client and such notification will be received through a call to
         \ref `.ClientListener.onPropertyChange` with argument "realMaxBandwidth" on any ClientListener listening
         to the related LightstreamerClient.

        @param maxBandwidth:  A decimal number, which represents the maximum bandwidth requested for the streaming or
         polling connection expressed in kbps (kilobits/sec). The string "unlimited" is also allowed, to mean that the
         maximum bandwidth can be entirely decided on the Server side (the check is case insensitive).

        @throws IllegalArgumentException: if a negative, zero, or a not-number value (excluding special values) is passed.

        .. seealso:: \ref `getRealMaxBandwidth`
        **/
        void setRequestedMaxBandwidth(MaxBandwidth maxbandwidth) {}

        /**
		Setter method that sets

        1. the minimum time to wait before trying a new connection to the Server in case the previous one failed for any reason; and
        2. the maximum time to wait for a response to a request before dropping the connection and trying with a different approach.


        Enforcing a delay between reconnections prevents strict loops of connection attempts when these attempts
        always fail immediately because of some persisting issue.
        This applies both to reconnections aimed at opening a new session and to reconnections
                aimed at attempting a recovery of the current session.

        Note that the delay is calculated from the moment the effort to create a connection
                is made, not from the moment the failure is detected.
        As a consequence, when a working connection is interrupted, this timeout is usually
                already consumed and the new attempt can be immediate (except that
        \ref `ConnectionOptions.setFirstRetryMaxDelay` will apply in this case).
        As another consequence, when a connection attempt gets no answer and times out,
        the new attempt will be immediate.

        As a timeout on unresponsive connections, it is applied in these cases:

        - \em Streaming: Applied on any attempt to setup the streaming connection. If after the timeout no data has
         arrived on the stream connection, the client may automatically switch transport or may resort to a polling
         connection.

        - \em "Polling and pre-flight requests": Applied on every connection. If after the timeout no data has arrived on
        the polling connection, the entire connection process restarts from scratch.


        @b "This setting imposes only a minimum delay. In order to avoid network congestion, the library may use a longer delay if the issue preventing the establishment of a session persists."

        @b default 4000 (4 seconds).

        @b lifecycle This value can be set and changed at any time.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "retryDelay" on any
        ClientListener listening to the related LightstreamerClient.

        @param retryDelay: The time (in milliseconds) to wait before trying a new connection.

        @throws IllegalArgumentException: if a negative or zero value is configured

        .. seealso:: \ref `setFirstRetryMaxDelay`
        **/
        void setRetryDelay(RetryDelay retrydelay) {}

        /**
		* Setter method that enables/disables the reverse-heartbeat mechanism by setting the heartbeat interval. If the given value (expressed in milliseconds) equals 0 then the reverse-heartbeat mechanism will be disabled; otherwise if the given value is greater than 0 the mechanism will be enabled with the specified interval.
        *
        * When the mechanism is active, the client will ensure that there is at most the specified interval between a control request and the following one, by sending empty control requests (the "reverse heartbeats") if necessary.
        *
        * This can serve various purposes:
        *
        * - Preventing the communication infrastructure from closing an inactive socket that is ready for reuse for more HTTP control requests, to avoid connection reestablishment overhead. However it is not guaranteed that the connection will be kept open,as the underlying TCP implementation may open a new socket each time a HTTP request needs to be sent. Note that this will be done only when a session is in place.
        * - Allowing the Server to detect when a streaming connection or Websocket is interrupted but not closed.  In these cases, the client eventually closes the connection, but the Server cannot see that (the connection remains "half-open") and just keeps trying to write. This is done by notifying the timeout to the Server upon each streaming request. For long polling, the \ref `setIdleTimeout` setting has a similar function.
        * - Allowing the Server to detect cases in which the client has closed a connection in HTTP streaming, but the socket is kept open by some intermediate node, which keeps consuming the response. This is also done by notifying the timeout to the Server upon each streaming request,whereas, for long polling, the \ref `setIdleTimeout` setting has a similar function.
        *
        * @b default 0 (meaning that the mechanism is disabled).
        *
        * @b lifecycle This setting should be performed before calling the
        *         \ref `LightstreamerClient.connect` method. However, the value can be changed
        *         at any time: the setting will be obeyed immediately, unless a higher heartbeat
        *         frequency was notified to the Server for the current connection. The setting
        * will always be obeyed upon the next connection (either a bind or a brand new session).
        *
        * @b notification A change to this setting will be notified through a call to
        *         \ref `.ClientListener.onPropertyChange` with argument "reverseHeartbeatInterval" on any
        * ClientListener listening to the related LightstreamerClient.
        *
        * @param reverseHeartbeatInterval: the interval, expressed in milliseconds, between subsequent reverse-heartbeats, or 0.
        *
        * @throws IllegalArgumentException: if a negative value is configured
        **/
        void setReverseHeartbeatInterval(ReverseHeartbeatInterval reverseheartbeatinterval) {}

        /**
		Setter method that can be used to disable/enable the automatic handling of server instance address that may
        be returned by the Lightstreamer server during session creation.

        In fact, when a Server cluster is in place, the Server address specified through
        \ref `ConnectionDetails.setServerAddress` can identify various Server instances; in order to
        ensure that all requests related to a session are issued to the same Server instance, the Server can answer
        to the session opening request by providing an address which uniquely identifies its own instance.

        Setting this value to true permits to ignore that address and to always connect through the address
        supplied in setServerAddress. This may be needed in a test environment, if the Server address specified
        is actually a local address to a specific Server instance in the cluster.


        **general edition note** Server Clustering is an optional feature, available depending on Edition and License Type.
        To know what features are enabled by your license, please see the License tab of the Monitoring Dashboard (by default,
        available at /dashboard).

        @b default false.

        @b lifecycle This method can be called at any time. If called while connected, it will be applied when the
        next session creation request is issued.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "serverInstanceAddressIgnored" on any
        ClientListener listening to the related LightstreamerClient.

        @param serverInstanceAddressIgnored: true or false, to ignore or not the server instance address sent by the server.

        .. seealso:: \ref `ConnectionDetails.setServerAddress`
        **/
        void setServerInstanceAddressIgnored(ServerInstanceAddressIgnored serverinstanceaddressignored) {}

        /**
		Setter method that sets the maximum time allowed for attempts to recover the current session upon an
        interruption, after which a new session will be created. If the given value (expressed in milliseconds)
        equals 0, then any attempt to recover the current session will be prevented in the first place.

        In fact, in an attempt to recover the current session, the client will
                periodically try to access the Server at the address related with the current
                session. In some cases, this timeout, by enforcing a fresh connection attempt,
                may prevent an infinite sequence of unsuccessful attempts to access the Server.

        Note that, when the Server is reached, the recovery may fail due to a
        Server side timeout on the retention of the session and the updates sent.
        In that case, a new session will be created anyway.
        A setting smaller than the Server timeouts may prevent such useless failures,
                but, if too small, it may also prevent successful recovery in some cases.

        @b default 15000 (15 seconds).

        @b lifecycle This value can be set and changed at any time.

        @b notification A change to this setting will be notified through a
        call to \ref `.ClientListener.onPropertyChange` with argument "sessionRecoveryTimeout" on any
        ClientListener listening to the related LightstreamerClient.

        @param sessionRecoveryTimeout: The maximum time allowed for recovery attempts, expressed in milliseconds,
        including 0.

        @throws IllegalArgumentException: if a negative value is passed.
        **/
        void setSessionRecoveryTimeout(SessionRecoveryTimeout sessionrecoverytimeout) {}

        /**
		Setter method that turns on or off the slowing algorithm. This heuristic algorithm tries to detect when the client
        CPU is not able to keep the pace of the events sent by the Server on a streaming connection. In that case, an automatic
        transition to polling is performed.

        In polling, the client handles all the data before issuing the next poll, hence a slow client would just delay the polls,
        while the Server accumulates and merges the events and ensures that no obsolete data is sent.

        Only in very slow clients, the next polling request may be so much delayed that the Server disposes the session first,
        because of its protection timeouts. In this case, a request for a fresh session will be reissued by the client and this
        may happen in cycle.

        @b default false.

        @b lifecycle This setting should be performed before
        calling the \ref `LightstreamerClient.connect` method. However, the value can be changed at any time: the supplied value will
        be used for the next streaming connection (either a bind or a brand new session).

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "slowingEnabled" on any
        ClientListener listening to the related LightstreamerClient.

        @param slowingEnabled: true or false, to enable or disable the heuristic algorithm that lowers the item update frequency.
        **/
        void setSlowingEnabled(SlowingEnabled slowingenabled) {}

        /**
		Setter method that sets the extra time the client is allowed to wait when an expected keepalive packet has not been 
        received on a stream connection (and no actual data has arrived), before entering the "STALLED" status.

        @b default 2000 (2 seconds).

        @b lifecycle  This value can be set and changed at any time.

        @b notification A change to this setting will be notified through a call to
                \ref `.ClientListener.onPropertyChange` with argument "stalledTimeout" on any
        ClientListener listening to the related LightstreamerClient.

        @param stalledTimeout: The idle time (in milliseconds) allowed before entering the "STALLED" status.

        @throws IllegalArgumentException: if a negative or zero value is configured

        .. seealso:: \ref `setReconnectTimeout`
        .. seealso:: \ref `setKeepaliveInterval`
        **/
        void setStalledTimeout(StalledTimeout stalledtimeout) {}

    };

}