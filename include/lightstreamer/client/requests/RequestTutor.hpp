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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTTUTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTTUTOR_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include "SessionThread.h"
#include "InternalConnectionOptions.h"
#include "Session.h"
#include "ServerSession.h"
#include <Logger.hpp>

namespace lightstreamer::client::requests {

    // Base definition for managing requests with retries and timeouts.
    class RequestTutor {
    protected:
        static inline Logger log = Logger::GetLogger("RequestsLog");
        static inline const long MIN_TIMEOUT = 4000; // The minimum retransmission timeout.
        long timeoutMs;
        std::shared_ptr<SessionThread> sessionThread;
        std::shared_ptr<InternalConnectionOptions> connectionOptions;
        std::shared_ptr<Session> session;
        std::shared_ptr<ServerSession> serverSession;
        bool timeoutIsRunning = false; // Flag to ensure only one timeout is pending at a time.
        bool discarded = false; // Flag to indicate if the tutor has been discarded.

    public:
        RequestTutor(std::shared_ptr<SessionThread> thread, std::shared_ptr<InternalConnectionOptions> options,
                     long currentTimeout = 0, bool fixedTO = false)
                : sessionThread(thread), connectionOptions(options) {
            session = sessionThread->GetSessionManager().GetSession();
            serverSession = sessionThread->GetSessionManager().GetServerSession();
            if (fixedTO) {
                timeoutMs = FixedTimeout();
            } else {
                timeoutMs = currentTimeout > 0 ? currentTimeout * 2 : MIN_TIMEOUT;
            }
        }

        virtual ~RequestTutor() = default;

        long GetTimeout() const {
            return timeoutMs;
        }

        // Notifies the tutor to start a timeout or perform a retry.
        void NotifySender(bool failed) {
            if (failed) {
                DoRecovery();
            } else {
                StartTimeout();
            }
        }

    protected:
        // Starts the timeout for the request retry.
        void StartTimeout() {
            if (!timeoutIsRunning) {
                timeoutIsRunning = true;
                sessionThread->Schedule([this]() { OnTimeout(); }, std::chrono::milliseconds(timeoutMs));
            }
        }

        void Discard() {
            discarded = true;
        }

        // Handles the timeout for the request retransmission.
        virtual void OnTimeout() {
            timeoutIsRunning = false;
            bool success = VerifySuccess();
            if (discarded || success) {
                return; // Stop retransmissions and discard tutor.
            } else if (serverSession->IsClosed()) {
                assert(!success);
                return; // Stop retransmissions and discard tutor.
            } else if (serverSession->IsTransportHttp() || dynamic_cast<Session::ForceRebindTutor *>(this) != nullptr) {
                assert(!success && serverSession->IsOpen());
                DoRecovery(); // Always retry when the transport is HTTP.
            } else if (!serverSession->IsSameStreamConnection(*session)) {
                assert(!success && serverSession->IsOpen() && serverSession->IsTransportWS());
                DoRecovery(); // Session has changed: retry transmission.
            } else {
                assert(!success && serverSession->IsOpen() && serverSession->IsTransportWS() &&
                       serverSession->IsSameStreamConnection(*session));
                if (!TimeoutFixed()) {
                    timeoutMs *= 2;
                }
                StartTimeout(); // Reschedule the tutor.
            }
        }

    public:
        virtual bool ShouldBeSent() const = 0;

        virtual bool VerifySuccess() const = 0;

        virtual void DoRecovery() = 0;

        // Called if the request will not be sent willingly (e.g., ADD not sent because a REMOVE was received before ADD was on the net).
        virtual void NotifyAbort() = 0;

    protected:
        virtual bool TimeoutFixed() const {
            return false;
        }

        virtual long FixedTimeout() const {
            return 0;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTTUTOR_HPP
