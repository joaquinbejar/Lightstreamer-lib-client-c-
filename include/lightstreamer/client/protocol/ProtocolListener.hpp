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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOLLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOLLISTENER_HPP

#include <vector>
#include <string>
#include "ConstrainTutor.h"

namespace lightstreamer::client::protocol {

    class ProtocolListener {
    public:
        virtual ~ProtocolListener() = default;

        virtual void onConstrainResponse(ConstrainTutor tutor) = 0;

        virtual void onServerSentBandwidth(const std::string& maxBandwidth) = 0;

        virtual void onTakeover(int specificCode) = 0;

        virtual void onExpiry() = 0;

        virtual void onKeepalive() = 0;

        virtual void onOKReceived(const std::string& newSession, const std::string& controlLink, long requestLimitLength, long keepaliveIntervalDefault) = 0;

        virtual void onLoopReceived(long serverSentPause) = 0;

        virtual void onSyncError(bool async) = 0;

        virtual void onUpdateReceived(int subscriptionId, int item, const std::vector<std::string>& values) = 0;

        virtual void onEndOfSnapshotEvent(int subscriptionId, int item) = 0;

        virtual void onClearSnapshotEvent(int subscriptionId, int item) = 0;

        virtual void onLostUpdatesEvent(int subscriptionId, int item, int lost) = 0;

        virtual void onMessageAck(const std::string& sequence, int messageNumber, bool async) = 0;

        virtual void onMessageOk(const std::string& sequence, int messageNumber) = 0;

        virtual void onMessageDeny(const std::string& sequence, int denyCode, const std::string& denyMessage, int messageNumber, bool async) = 0;

        virtual void onMessageDiscarded(const std::string& sequence, int messageNumber, bool async) = 0;

        virtual void onMessageError(const std::string& sequence, int errorCode, const std::string& errorMessage, int messageNumber, bool async) = 0;

        virtual void onSubscriptionError(int subscriptionId, int errorCode, const std::string& errorMessage, bool async) = 0;

        virtual void onServerError(int errorCode, const std::string& errorMessage) = 0;

        virtual void onUnsubscription(int subscriptionId) = 0;

        virtual void onSubscription(int subscriptionId, int totalItems, int totalFields, int keyPosition, int commandPosition) = 0;

        virtual void onSubscriptionReconf(int subscriptionId, long reconfId, bool async) = 0;

        virtual void onSyncMessage(long seconds) = 0;

        virtual void onInterrupted(bool wsError, bool unableToOpen) = 0;

        virtual void onConfigurationEvent(int subscriptionId, const std::string& frequency) = 0;

        virtual void onServerName(const std::string& serverName) = 0;

        virtual void onClientIp(const std::string& clientIp) = 0;

        virtual void onSubscriptionAck(int subscriptionId) = 0;

        virtual void onUnsubscriptionAck(int subscriptionId) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnRegisterOK(const std::string& deviceId, const std::string& adapterName) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnRegisterError(int code, const std::string& message) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnSubscribeOK(const std::string& lsSubId, const std::string& pnSubId) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnSubscribeError(const std::string& subId, int code, const std::string& message) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnUnsubscribeError(const std::string& subId, int code, const std::string& message) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnUnsubscribeOK(const std::string& subId) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnResetBadgeOK(const std::string& deviceId) = 0;

        // Forwards the MPN event to the Session Manager.
        virtual void onMpnBadgeResetError(int code, const std::string& message) = 0;

        virtual long getDataNotificationProg() const = 0;

        virtual void onDataNotification() = 0;

        virtual void onRecoveryError() = 0;
    };

} // namespace lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_PROTOCOLLISTENER_HPP
