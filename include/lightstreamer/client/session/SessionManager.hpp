/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 18/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/session/SessionListener.hpp>
#include <lightstreamer/client/session/SessionsListener.hpp>
#include <lightstreamer/client/session/SessionFactory.hpp>
#include <lightstreamer/client/session/Session.hpp>
#include <lightstreamer/client/session/ServerSession.hpp>
#include <Logger.hpp>


namespace lightstreamer::client::session {

    /**
     * Manages sessions, implementing the SessionListener interface.
     */
    class SessionManager : public SessionListener {

        static std::shared_ptr<SessionFactory> sessionFactory;
    public:
        /**
         * Allows setting a custom session factory.
         * Note: This setter is intended just for testing.
         */
        static void setCustomFactory(std::shared_ptr<SessionFactory> value) {
            sessionFactory = value;
        }

        enum class Status {
            OFF,
            STREAMING_WS,
            SWITCHING_STREAMING_WS,
            POLLING_WS,
            SWITCHING_POLLING_WS,
            STREAMING_HTTP,
            SWITCHING_STREAMING_HTTP,
            POLLING_HTTP,
            SWITCHING_POLLING_HTTP,
            END,
            ERROR
        };

        static constexpr bool STREAMING_SESSION = false;
        static constexpr bool POLLING_SESSION = true;
        static constexpr bool WS_SESSION = false;
        static constexpr bool HTTP_SESSION = true;

        static constexpr bool AVOID_SWITCH = true;
        //static constexpr bool NO_RECOVERY = true;
        static constexpr bool YES_RECOVERY = false;

        /**
         * Converts a status enum to a descriptive string.
         */
        static std::string statusToString(Status type) {
            if (type == Status::ERROR) {
                return {};
            }

            switch (type) {
                case Status::OFF:
                    return "No session";
                case Status::STREAMING_WS:
                    return "WS Streaming";
                case Status::SWITCHING_STREAMING_WS:
                    return "prepare WS Streaming";
                case Status::POLLING_WS:
                    return "WS Polling";
                case Status::SWITCHING_POLLING_WS:
                    return "prepare WS Polling";
                case Status::STREAMING_HTTP:
                    return "HTTP Streaming";
                case Status::SWITCHING_STREAMING_HTTP:
                    return "prepare HTTP Streaming";
                case Status::POLLING_HTTP:
                    return "HTTP Polling";
                case Status::SWITCHING_POLLING_HTTP:
                    return "prepare HTTP Polling";
                case Status::END:
                    return "Shutting down";
                default:
                    return "Error";
            }
        }

        std::shared_ptr<ILogger> log;
        Status status = Status::OFF;
        int statusPhase = 0;
        std::shared_ptr<Session> session;
        std::shared_ptr<ServerSession> serverSession;
        bool isFrozen = false;
        std::string clientIP;
        std::shared_ptr<InternalConnectionOptions> options;
        std::shared_ptr<InternalConnectionDetails> details;
        std::shared_ptr<SubscriptionsListener> subscriptions;
        std::shared_ptr<MessagesListener> messages;
        std::shared_ptr<SessionsListener> listener;
        SessionThread thread;

        /**
         * Counts the bind_session requests following the corresponding create_session.
         */
        int nBindAfterCreate = 0;

        /**
         * Constructs a SessionManager with basic session components.
         * @param options Options for the internal connection.
         * @param details Details of the internal connection.
         * @param thread Thread handling session operations.
         */
        SessionManager(std::shared_ptr<InternalConnectionOptions> options, std::shared_ptr<InternalConnectionDetails> details, SessionThread& thread)
                : options(options), details(details), thread(thread) {
        }

        /**
         * Constructs a SessionManager with listeners for testing.
         * This constructor initializes session management with various listeners.
         * @param options Options for the internal connection.
         * @param details Details of the internal connection.
         * @param listener Session event listener.
         * @param subscriptions Subscription event listener.
         * @param messages Message event listener.
         * @param thread Thread handling session operations.
         */
        SessionManager(std::shared_ptr<InternalConnectionOptions> options, std::shared_ptr<InternalConnectionDetails> details, std::shared_ptr<SessionsListener> listener, std::shared_ptr<SubscriptionsListener> subscriptions, std::shared_ptr<MessagesListener> messages, SessionThread& thread)
                : SessionManager(options, details, thread) {
            this->subscriptions = subscriptions;
            this->messages = messages;
            this->listener = listener;
        }

        /**
         * Sets the session event listener.
         * @param value The new session listener to be used.
         */
        void setSessionsListener(std::shared_ptr<SessionsListener> value) {
            listener = value;
        }

        /**
         * Sets the subscription event listener.
         * @param value The new subscriptions listener to be used.
         */
        void setSubscriptionsListener(std::shared_ptr<SubscriptionsListener> value) {
            subscriptions = value;
        }

        /**
         * Sets the message event listener.
         * @param value The new messages listener to be used.
         */
        void setMessagesListener(std::shared_ptr<MessagesListener> value) {
            messages = value;
        }

    private:
        /**
         * Changes the current status of the session, logging the change if debugging is enabled.
         * @param newStatus The new status to set.
         */
        void changeStatus(Status newStatus) {
            if (log->isDebugEnabled()) {
                log->debug("SessionManager state change: " + std::to_string(static_cast<int>(status)) + " -> " + std::to_string(static_cast<int>(newStatus)));
            }
            status = newStatus;
            statusPhase++;
        }

        /**
 * Checks if the current status matches the given status.
 * @param check The status to check against the current status.
 * @return true if the current status matches the given status, false otherwise.
 */
        bool is(Status check) const {
            return status == check;
        }

        /**
         * Checks if the current status does not match the given status.
         * @param check The status to check against the current status.
         * @return true if the current status does not match the given status, false otherwise.
         */
        bool isNot(Status check) const {
            return !is(check);
        }

        /**
         * Checks if the session is alive, meaning it is neither OFF nor END.
         * @return true if the session is alive, false otherwise.
         */
        bool isAlive() const {
            return isNot(Status::OFF) && isNot(Status::END);
        }

        /**
         * Determines the next status for the sense phase based on the current status and conditions.
         * @return The next status appropriate for the sense phase.
         */
        Status getNextSensePhase() const {
            switch (status) {
                case Status::STREAMING_WS:
                    return isFrozen ? Status::SWITCHING_STREAMING_WS : Status::SWITCHING_STREAMING_HTTP;

                case Status::STREAMING_HTTP:
                    return Status::SWITCHING_POLLING_HTTP;

                case Status::POLLING_WS:
                    return Status::SWITCHING_STREAMING_WS;

                case Status::POLLING_HTTP:
                    return isFrozen ? Status::SWITCHING_POLLING_HTTP : Status::SWITCHING_STREAMING_WS;

                default: // already switching
                    return status;
            }
        }

        /**
         * Determines the next status for the slow phase based on the current status.
         * @return The next status appropriate for the slow phase or ERROR if an invalid status transition is requested.
         */
        Status getNextSlowPhase() const {
            switch (status) {
                case Status::STREAMING_WS:
                    return Status::SWITCHING_POLLING_WS;

                case Status::STREAMING_HTTP:
                case Status::SWITCHING_STREAMING_HTTP:
                case Status::SWITCHING_POLLING_HTTP:
                    return Status::SWITCHING_POLLING_HTTP;

                default: // already switching
                    return Status::ERROR;
            }
        }

        /**
         * Retrieves the session ID if a session is present, or returns an empty string if no session is present.
         * @return The session ID or an empty string.
         */
        std::string getSessionId() const {
            return session ? session->getSessionId() : "";
        }

        /**
         * Prepares a new session instance, potentially replacing an existing session.
         * @param isPolling Indicates if polling mode should be used.
         * @param isComboForced Indicates if combo is forced.
         * @param isHTTP Indicates if HTTP is used.
         * @param prevSession The previous session, which may be replaced by the new one.
         * @param retryAgainIfStreamFails Indicates if retries should be made if streaming fails.
         * @param sessionRecovery Indicates if session recovery should be initiated.
         */
        void prepareNewSessionInstance(bool isPolling, bool isComboForced, bool isHTTP, std::shared_ptr<Session> prevSession, bool retryAgainIfStreamFails, bool sessionRecovery) {
            session = sessionFactory->createNewSession(isPolling, isComboForced, isHTTP, prevSession, shared_from_this(), subscriptions, messages, thread, details, options, statusPhase, retryAgainIfStreamFails, sessionRecovery);

            if (!prevSession) {
                if (serverSession) {
                    serverSession->close();
                }
                serverSession = std::make_shared<ServerSession>(session);
            } else {
                serverSession->setNewStreamConnection(session);
            }

            if (prevSession) {
                prevSession->shutdown(false);
            }
        }

        /**
         * Binds a session, optionally with forced parameters and initiates a session with specific protocols.
         * @param isForced Forces the session creation disregarding normal session rules.
         * @param isPolling Indicates if polling mode should be used.
         * @param isHTTP Indicates if HTTP is used.
         * @param switchCause Reason for switching sessions.
         * @param startRecovery Indicates if recovery should be initiated.
         */
        void bindSession(bool isForced, bool isPolling, bool isHTTP, const std::string& switchCause, bool startRecovery) {
            Status nextPH = isPolling ? (isHTTP ? Status::POLLING_HTTP : Status::POLLING_WS) : (isHTTP ? Status::STREAMING_HTTP : Status::STREAMING_WS);
            changeStatus(nextPH);
            prepareNewSessionInstance(isPolling, isForced, isHTTP, session, false, startRecovery);
            session->bindSession(switchCause);
        }

        /**
         * Starts a timeout for switching sessions, considering any potential delays.
         * @param reason The reason for the timeout.
         * @param delay Additional delay to apply.
         */
        void startSwitchTimeout(const std::string& reason, long delay) {
            long timeout = options->getSwitchCheckTimeout() + delay;
            int ph = statusPhase;

            std::thread([this, ph, reason, timeout]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
                switchTimeout(ph, reason);
            }).detach();
        }



    public:

        /**
 * Handles non-recoverable errors, closing the session if necessary.
 * @param e The exception representing the error.
 */
        virtual void onFatalError(const std::exception& e) {
            if (session) {
                session->onFatalError(e);
            }
        }

        /**
         * Adjusts the bandwidth constraints for the session.
         */
        virtual void changeBandwidth() {
            if (session) {
                session->sendConstrain(0, nullptr);
            }
        }

        /**
         * Handles a reverse heartbeat, used to keep the connection alive.
         * @param force Whether to force the heartbeat.
         */
        virtual void handleReverseHeartbeat(bool force) {
            if (session) {
                session->handleReverseHeartbeat(force);
            }
        }

        /**
         * Closes the session.
         * @param fromAPI Indicates if the request originated from the API.
         * @param reason The reason for closing the session.
         * @param noRecoveryScheduled Indicates if no recovery should be attempted.
         */
        virtual void closeSession(bool fromAPI, const std::string& reason, bool noRecoveryScheduled) {
            try {
                if (noRecoveryScheduled && serverSession) {
                    serverSession->close();
                }
                if (is(Status::OFF) || is(Status::END)) {
                    return;
                }
                if (session) {
                    std::string closeReason = fromAPI ? "api" : reason;
                    session->closeSession(closeReason, false, noRecoveryScheduled);
                }
            } catch (const std::exception& e) {
                log->warn("Something went wrong here ::: " + std::string(e.what()));
                if (log->isDebugEnabled()) {
                    log->debug("Exception details: " + std::string(e.what())); // Adjust based on actual logging capabilities
                }
            }
        }

        /**
         * Creates a new session with specified properties.
         * @param fromAPI Indicates if the creation request originated from the API.
         * @param isTransportForced Indicates if transport type is forced.
         * @param isComboForced Indicates if combo is forced.
         * @param isPolling Indicates if polling mode should be used.
         * @param isHTTP Indicates if HTTP is used.
         * @param reason The reason for creating the session.
         * @param avoidSwitch Indicates if switching should be avoided.
         * @param retryAgainIfStreamFails Indicates if retries should be made if streaming fails.
         * @param startRecovery Indicates if recovery should be started.
         */
        virtual void createSession(bool fromAPI, bool isTransportForced, bool isComboForced, bool isPolling, bool isHTTP, std::string reason, bool avoidSwitch, bool retryAgainIfStreamFails, bool startRecovery) {
            reason = fromAPI ? "api" : reason;
            isFrozen = isTransportForced;

            if (!avoidSwitch && isAlive()) {
                Status nextPH = isPolling ? (isHTTP ? Status::SWITCHING_POLLING_HTTP : Status::SWITCHING_POLLING_WS) : (isHTTP ? Status::SWITCHING_STREAMING_HTTP : Status::SWITCHING_STREAMING_WS);
                changeStatus(nextPH);
                // startSwitchTimeout(reason, 0); // Need to implement or adapt this method if it's part of your C++ code
                session->requestSwitch(statusPhase, reason, isComboForced, startRecovery);
            } else {
                std::string currSessionId = session ? session->getSessionId() : "";
                reason = "new." + reason;
                closeSession(false, reason, YES_RECOVERY);
                Status nextPH = isPolling ? (isHTTP ? Status::POLLING_HTTP : Status::POLLING_WS) : (isHTTP ? Status::STREAMING_HTTP : Status::STREAMING_WS);
                changeStatus(nextPH);
                // prepareNewSessionInstance(isPolling, isComboForced, isHTTP, nullptr, retryAgainIfStreamFails, false); // Need to implement or adapt this method if it's part of your C++ code
                session->createSession(currSessionId, reason);
            }
        }

        /**
         * Attempts to retry a session connection based on the current phase of the handler.
         * @param handlerPhase Current phase of the session handler.
         * @param retryCause The reason for the retry.
         * @param forced Indicates if the retry is forced.
         * @param retryAgainIfStreamFails Indicates if retries should be made if streaming fails.
         */
        void retry(int handlerPhase, const std::string& retryCause, bool forced, bool retryAgainIfStreamFails) {
            if (handlerPhase != statusPhase) {
                return;
            }

            bool strOrPoll = is(Status::STREAMING_WS) || is(Status::STREAMING_HTTP) ? STREAMING_SESSION : POLLING_SESSION;
            bool wsOrHttp = is(Status::STREAMING_WS) || is(Status::POLLING_WS) ? WS_SESSION : HTTP_SESSION;

            createSession(false, isFrozen, forced, strOrPoll, wsOrHttp, retryCause, AVOID_SWITCH, retryAgainIfStreamFails, false);
        }

        /**
         * Similar to {@code bindSession} but with session recovery set to true.
         * A special bind_session request is sent during the recovery process.
         * @param handlerPhase Current phase of the session handler.
         * @param retryCause The reason for the recovery.
         * @param forced Indicates if the recovery is forced.
         * @param retryAgainIfStreamFails Indicates if retries should be made if streaming fails.
         */
        void recoverSession(int handlerPhase, const std::string& retryCause, bool forced, bool retryAgainIfStreamFails) {
            if (handlerPhase != statusPhase) {
                return;
            }

            bool isPolling = is(Status::STREAMING_WS) || is(Status::STREAMING_HTTP) ? STREAMING_SESSION : POLLING_SESSION;
            bool isHTTP = is(Status::STREAMING_WS) || is(Status::POLLING_WS) ? WS_SESSION : HTTP_SESSION;
            Status nextPH = isPolling ? (isHTTP ? Status::POLLING_HTTP : Status::POLLING_WS) : (isHTTP ? Status::STREAMING_HTTP : Status::STREAMING_WS);
            changeStatus(nextPH);

            prepareNewSessionInstance(isPolling, forced, isHTTP, session, retryAgainIfStreamFails, true);
            session->recoverSession();
        }



    };


}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
