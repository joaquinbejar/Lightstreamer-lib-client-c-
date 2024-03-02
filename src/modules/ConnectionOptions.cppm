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

    typedef void ContentLength; // TODO: Implement this type
    typedef void FirstRetryMaxDelay; // TODO: Implement this type
    typedef void ForcedTransport; // TODO: Implement this type
    typedef void HttpExtraHeaders; // TODO: Implement this type
    typedef void HttpExtraHeadersOnSessionCreationOnly; // TODO: Implement this type
    typedef void IdleTimeout; // TODO: Implement this type
    typedef void KeepaliveInterval; // TODO: Implement this type
    typedef void PollingInterval; // TODO: Implement this type
    typedef void Proxy; // TODO: Implement this type
    typedef void ReconnectTimeout; // TODO: Implement this type
    typedef void MaxBandwidth; // TODO: Implement this type
    typedef void RetryDelay; // TODO: Implement this type
    typedef void ReverseHeartbeatInterval; // TODO: Implement this type
    typedef void ServerInstanceAddressIgnored; // TODO: Implement this type
    typedef void SessionRecoveryTimeout; // TODO: Implement this type
    typedef void SlowingEnabled; // TODO: Implement this type
    typedef void StalledTimeout; // TODO: Implement this type


    class ConnectionOptions {

        void getContentLength() {}

        void getFirstRetryMaxDelay() {}

        void getForcedTransport() {}

        void getHttpExtraHeaders() {}

        void getIdleTimeout() {}

        void getKeepaliveInterval() {}

        void getPollingInterval() {}

        void getRealMaxBandwidth() {}

        void getReconnectTimeout() {}

        void getRequestedMaxBandwidth() {}

        void getRetryDelay() {}

        void getReverseHeartbeatInterval() {}

        void getSessionRecoveryTimeout() {}

        void getStalledTimeout() {}

        void isHttpExtraHeadersOnSessionCreationOnly() {}

        void isServerInstanceAddressIgnored() {}

        void isSlowingEnabled() {}

        void setContentLength(ContentLength contentlength) {}

        void setFirstRetryMaxDelay(FirstRetryMaxDelay firstretrymaxdelay) {}

        void setForcedTransport(ForcedTransport forcedtransport) {}

        void setHttpExtraHeaders(HttpExtraHeaders httpextraheaders) {}

        void setHttpExtraHeadersOnSessionCreationOnly(
                HttpExtraHeadersOnSessionCreationOnly httpextraheadersonsessioncreationonly) {}

        void setIdleTimeout(IdleTimeout idletimeout) {}

        void setKeepaliveInterval(KeepaliveInterval keepaliveinterval) {}

        void setPollingInterval(PollingInterval pollinginterval) {}

        void setProxy(Proxy proxy) {}

        void setReconnectTimeout(ReconnectTimeout reconnecttimeout) {}

        void setRequestedMaxBandwidth(MaxBandwidth maxbandwidth) {}

        void setRetryDelay(RetryDelay retrydelay) {}

        void setReverseHeartbeatInterval(ReverseHeartbeatInterval reverseheartbeatinterval) {}

        void setServerInstanceAddressIgnored(ServerInstanceAddressIgnored serverinstanceaddressignored) {}

        void setSessionRecoveryTimeout(SessionRecoveryTimeout sessionrecoverytimeout) {}

        void setSlowingEnabled(SlowingEnabled slowingenabled) {}

        void setStalledTimeout(StalledTimeout stalledtimeout) {}

    };

}