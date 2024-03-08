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

/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 7/3/24
 ******************************************************************************/

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <unordered_set>

namespace lightstreamer::client {

class Constants {
public:
    static constexpr auto TLCP_VERSION = "TLCP-2.1.0";
    static constexpr auto ACTIONS_LOG = "lightstreamer.actions";
    static constexpr auto SESSION_LOG = "lightstreamer.session";
    static constexpr auto SUBSCRIPTIONS_LOG = "lightstreamer.subscribe";
    static constexpr auto PROTOCOL_LOG = "lightstreamer.protocol";
    static constexpr auto TRANSPORT_LOG = "lightstreamer.stream";
    static constexpr auto THREADS_LOG = "lightstreamer.threads";
    // static constexpr auto NETTY_LOG = "lightstreamer.netty";
    // static constexpr auto NETTY_POOL_LOG = "lightstreamer.netty.pool";
    static constexpr auto REQUESTS_LOG = "lightstreamer.requests";
    static constexpr auto UTILS_LOG = "lightstreamer.utils";
    static constexpr auto MPN_LOG = "lightstreamer.mpn";
    static constexpr auto HEARTBEAT_LOG = "lightstreamer.heartbeat";
    static constexpr auto STATS_LOG = "lightstreamer.statistics";

    static constexpr auto UNLIMITED = "unlimited";
    static constexpr auto AUTO = "auto";

    static inline std::unordered_set<std::string> FORCED_TRANSPORTS = {"HTTP", "HTTP-POLLING", "HTTP-STREAMING", "WS", "WS-POLLING", "WS-STREAMING", ""};

    static constexpr auto COMMAND = "COMMAND";
    static constexpr auto RAW = "RAW";
    static constexpr auto MERGE = "MERGE";
    static constexpr auto DISTINCT = "DISTINCT";

    static constexpr auto UNORDERED_MESSAGES = "UNORDERED_MESSAGES";

    static inline std::unordered_set<std::string> MODES = {COMMAND, MERGE, DISTINCT, RAW};

    static inline std::unordered_set<std::string> PROXY_TYPES = {"HTTP"}; // "SOCKS4", "SOCKS5"

    static constexpr auto DISCONNECTED = "DISCONNECTED";
    static constexpr auto WILL_RETRY = "DISCONNECTED:WILL-RETRY";
    static constexpr auto TRYING_RECOVERY = "DISCONNECTED:TRYING-RECOVERY";
    static constexpr auto CONNECTING = "CONNECTING";
    static constexpr auto CONNECTED = "CONNECTED:";
    static constexpr auto STALLED = "STALLED";
    static constexpr auto HTTP_POLLING = "HTTP-POLLING";
    static constexpr auto HTTP_STREAMING = "HTTP-STREAMING";
    static constexpr auto SENSE = "STREAM-SENSING";
    static constexpr auto WS_STREAMING = "WS-STREAMING";
    static constexpr auto WS_POLLING = "WS-POLLING";

    static constexpr auto WS_ALL = "WS";
    static constexpr auto HTTP_ALL = "HTTP";

    static constexpr auto DELETE = "DELETE";
    static constexpr auto UPDATE = "UPDATE";
    static constexpr auto ADD = "ADD";

    static constexpr int CLOSE_SOCKET_TIMEOUT_MILLIS = 5000;
};

}

#endif //CONSTANTS_HPP
