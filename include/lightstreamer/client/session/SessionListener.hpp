/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 17/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONLISTENER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONLISTENER_HPP
#pragma once

#include <string>
#include <memory>

namespace lightstreamer::client::session {

    /**
     * Interface for session listener.
     */
    class SessionListener {
    public:
        virtual ~SessionListener() = default;

        virtual void sessionStatusChanged(int handlerPhase, const std::string& phase, bool sessionRecovery) = 0;

        virtual void streamSense(int handlerPhase, const std::string& switchCause, bool forced) = 0;

        virtual void switchReady(int handlerPhase, const std::string& switchCause, bool forced, bool startRecovery) = 0;

        virtual void slowReady(int handlerPhase) = 0;

        virtual int onSessionClose(int handlerPhase, bool noRecoveryScheduled) = 0;

        virtual void streamSenseSwitch(int handlerPhase, const std::string& reason, const std::string& sessionPhase, bool startRecovery) = 0;

        virtual void onIPReceived(const std::string& clientIP) = 0;

        virtual void onSessionBound() = 0;

        virtual void onSessionStart() = 0;

        virtual void onServerError(int errorCode, const std::string& errorMessage) = 0;

        virtual void onSlowRequired(int handlerPhase, long delay) = 0;

        virtual void retry(int handlerPhase, const std::string& retryCause, bool forced, bool retryAgainIfStreamFails) = 0;

        /**
         * Since the client IP has changed and WebSocket support
         * was enabled again, we ask the SessionManager
         * to use a WebSocket transport for this session.
         */
        virtual void switchToWebSocket(bool startRecovery) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnRegisterOK(const std::string& deviceId, const std::string& adapterName) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnRegisterError(int code, const std::string& message) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnSubscribeOK(const std::string& lsSubId, const std::string& pnSubId) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnSubscribeError(const std::string& subId, int code, const std::string& message) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnUnsubscribeError(const std::string& subId, int code, const std::string& message) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnUnsubscribeOK(const std::string& subId) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnResetBadgeOK(const std::string& deviceId) = 0;

        /**
         * Forward the MPN event to the MPN manager.
         */
        virtual void onMpnBadgeResetError(int code, const std::string& message) = 0;

        virtual void recoverSession(int handlerPhase, const std::string& retryCause, bool forced, bool retryAgainIfStreamFails) = 0;
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONLISTENER_HPP
