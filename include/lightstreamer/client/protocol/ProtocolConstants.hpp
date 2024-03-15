/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 15/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOLCONSTANTS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOLCONSTANTS_HPP

#include <string>

namespace lightstreamer::client::protocol {

    // Constants used throughout the Lightstreamer client protocol
    class ProtocolConstants {
    public:
        // Answer sent by the Server to signal accepted requests.
        inline static const std::string conokCommand = "CONOK";

        // Constant sent by the Server in case no data has been sent for a configured time.
        inline static const std::string probeCommand = "PROBE";

        // Constant sent by the Server before closing the connection due to the content length consumption.
        inline static const std::string loopCommand = "LOOP";

        // Constant sent by the Server before closing the connection because of a server-side decision.
        inline static const std::string endCommand = "END";

        // Answer sent by the Server to signal the refusal of a request.
        inline static const std::string conerrCommand = "CONERR";

        // End of snapshot marker.
        inline static const std::string endOfSnapshotMarker = "EOS";

        // Overflow notification marker.
        inline static const std::string overflowMarker = "OV";

        // Clear-snapshot notification marker.
        inline static const std::string clearSnapshotMarker = "CS,";

        // Message notification marker.
        inline static const std::string msgMarker = "MSG";

        inline static const std::string subscribeMarker = "SUB";
        inline static const std::string unsubscribeMarker = "UNSUB,";
        inline static const std::string constrainMarker = "CONS,";
        inline static const std::string syncMarker = "SYNC,";
        inline static const std::string updateMarker = "U,";
        inline static const std::string configurationMarker = "CONF,";
        inline static const std::string serverNameMarker = "SERVNAME,";
        inline static const std::string clientIpMarker = "CLIENTIP,";
        inline static const std::string progMarker = "PROG,";
        inline static const std::string noopMarker = "NOOP,";
        inline static const std::string reqokMarker = "REQOK,";
        inline static const std::string reqerrMarker = "REQERR,";
        inline static const std::string errorMarker = "ERROR,";
        inline static const std::string mpnRegisterMarker = "MPNREG,";
        inline static const std::string mpnSubscribeMarker = "MPNOK,";
        inline static const std::string mpnUnsubscribeMarker = "MPNDEL,";
        inline static const std::string mpnResetBadgeMarker = "MPNZERO,";
        inline static const std::string UNCHANGED = "UNCHANGED";

        inline static const bool SYNC_RESPONSE = false;
        inline static const bool ASYNC_RESPONSE = true;

        inline static const std::string END_LINE = "\r\n";
    };

} // namespace lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOLCONSTANTS_HPP
