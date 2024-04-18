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

        /**
         * Handles a timeout for switching sessions, and attempts to create a new session if necessary.
         * @param ph Phase of status when the timeout was started.
         * @param reason Reason for the timeout.
         */
        void switchTimeout(int ph, const std::string& reason) {
            if (ph != statusPhase) {
                return;
            }

            log->info("Failed to switch session type. Starting new session " + statusToString(status));

            Status switchType = status;

            if (isNot(Status::SWITCHING_STREAMING_WS) && isNot(Status::SWITCHING_STREAMING_HTTP) &&
                isNot(Status::SWITCHING_POLLING_HTTP) && isNot(Status::SWITCHING_POLLING_WS)) {
                log->error("Unexpected fallback type switching because of a failed force rebind");
                return;
            }

            std::string timeoutReason = "switch.timeout." + reason;
            bool strOrPoll = switchType == Status::SWITCHING_STREAMING_WS || switchType == Status::SWITCHING_STREAMING_HTTP ? STREAMING_SESSION : POLLING_SESSION;
            bool wsOrHttp = switchType == Status::SWITCHING_STREAMING_WS || switchType == Status::SWITCHING_POLLING_WS ? WS_SESSION : HTTP_SESSION;

            createSession(false, isFrozen, false, strOrPoll, wsOrHttp, timeoutReason, AVOID_SWITCH, false, false);
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

        /**
         * Senses if a session switch is necessary due to issues with the current session and initiates the switch.
         * @param handlerPhase The current phase of the handler.
         * @param reason Reason for the switch.
         * @param sessionPhase Phase of the session, particularly important for handling first bindings.
         * @param startRecovery Indicates if recovery should start.
         */
        void streamSenseSwitch(int handlerPhase, const std::string& reason, const std::string& sessionPhase, bool startRecovery) {
            if (handlerPhase != statusPhase) {
                return;
            }

            Status switchType = getNextSensePhase();

            if (switchType == Status::OFF || switchType == Status::END) {
                log->warn("Unexpected fallback type switching with new session");
                return;
            }

            log->info("Unable to establish session of the current type. Switching session type " + statusToString(status) + "->" + statusToString(switchType));

            if (sessionPhase == "FIRST_BINDING" && status == Status::STREAMING_WS && switchType == Status::SWITCHING_STREAMING_HTTP) {
                log->info("WebSocket support has been disabled.");
                WebSocket::disable();
            }

            changeStatus(switchType);
            startSwitchTimeout(reason, 0);
            session->requestSwitch(statusPhase, reason, false, startRecovery);
        }

        /**
         * Responds to a slow connection detection by attempting to switch the session type accordingly.
         * @param handlerPhase The current phase of the handler.
         * @param delay Delay to apply to the switch timeout.
         */
        void onSlowRequired(int handlerPhase, long delay) {
            if (handlerPhase != statusPhase) {
                return;
            }

            Status switchType = getNextSlowPhase();

            log->info("Slow session detected. Switching session type " + statusToString(status) + "->" + statusToString(switchType));

            if (switchType == Status::ERROR) {
                log->error("Unexpected fallback type; switching because of a slow connection was detected" + statusToString(status) + ", " + session->toString());
                return;
            }

            changeStatus(switchType);
            startSwitchTimeout("slow", delay);
            session->requestSlow(statusPhase);
        }

        /**
         * Notifies about the change in session status.
         * @param handlerPhase The phase of the handler when the event was triggered.
         * @param phase The current phase description.
         * @param sessionRecovery Indicates if the session recovery mechanism is active.
         */
        void sessionStatusChanged(int handlerPhase, const std::string& phase, bool sessionRecovery) {
            if (log->isDebugEnabled()) {
                log->debug("sessionStatusChanged: " + std::to_string(handlerPhase) + " = " + std::to_string(statusPhase));
            }

            if (handlerPhase != statusPhase) {
                return;
            }
            listener->onStatusChanged(getHighLevelStatus(sessionRecovery));
        }

        /**
         * Handles the reception of a new client IP. If the IP has changed and WebSocket was disabled, restores it.
         * @param newIP The newly received IP address.
         */
        void onIPReceived(const std::string& newIP) {
            if (clientIP && newIP != clientIP && WebSocket::isDisabled()) {
                WebSocket::restore();
                session->restoreWebSocket();
            }
            clientIP = newIP;
        }

        /**
         * Called when a session is successfully bound.
         */
        void onSessionBound() {
            if (nBindAfterCreate == 0) {
                // mpnEventManager.onSessionStart(); // Uncomment or modify as necessary
            }
            nBindAfterCreate++;
        }

        /**
         * Called at the start of a session.
         */
        void onSessionStart() {
            nBindAfterCreate = 0;
        }

        /**
         * Notifies about a server-side error.
         * @param errorCode The error code received from the server.
         * @param errorMessage The error message received from the server.
         */
        void onServerError(int errorCode, const std::string& errorMessage) {
            listener->onServerError(errorCode, errorMessage);
        }

        /**
         * Handles session closure.
         * @param handlerPhase The phase of the handler when the session was closed.
         * @param noRecoveryScheduled Indicates whether a recovery was scheduled.
         * @return The status phase after processing the closure.
         */
        int onSessionClose(int handlerPhase, bool noRecoveryScheduled) {
            if (handlerPhase != statusPhase) {
                return 0;
            }

            log->debug("Session closed: " + getSessionId());

            if (noRecoveryScheduled) {
                changeStatus(Status::OFF);
            } else {
                changeStatus(status); // to change the statusPhase
            }

            // mpnEventManager.onSessionClose(!noRecoveryScheduled); // Uncomment or modify as necessary

            return statusPhase;
        }

        /**
         * Retrieves the high-level status of the session, indicating if it's connected, disconnected, etc.
         * @param sessionRecovery Indicates if session recovery is being considered.
         * @return The high-level status as a string.
         */
        std::string getHighLevelStatus(bool sessionRecovery) {
            std::string hlStatus = session == nullptr ? Constants::DISCONNECTED : session->getHighLevelStatus(sessionRecovery);
            return hlStatus;
        }

        /**
         * Accessor for the current session.
         * @return A pointer to the current Session object, which may be null.
         */
        std::shared_ptr<Session> getSession() const {
            return session;
        }

        /**
         * Accessor for the current server session.
         * @return A pointer to the current ServerSession object.
         */
        std::shared_ptr<ServerSession> getServerSession() const {
            return serverSession;
        }

        /**
         * Sends a message using the current session.
         * @param request The message request to send.
         * @param tutor An object that manages the reliability and retry logic for the request.
         */
        void sendMessage(std::shared_ptr<MessageRequest> request, std::shared_ptr<requests::RequestTutor> tutor) {
            if (session != nullptr) {
                session->sendMessage(request, tutor);
            }
        }

        /**
         * Sends a subscription request using the current session.
         * @param request The subscription request to send.
         * @param tutor An object that manages the reliability and retry logic for the request.
         */
        void sendSubscription(std::shared_ptr<SubscribeRequest> request, std::shared_ptr<requests::RequestTutor> tutor) {
            if (session != nullptr) {
                session->sendSubscription(request, tutor);
            }
        }

        /**
         * Sends an unsubscription request using the current session.
         * @param request The unsubscription request to send.
         * @param tutor An object that manages the reliability and retry logic for the request.
         */
        void sendUnsubscription(std::shared_ptr<UnsubscribeRequest> request, std::shared_ptr<requests::RequestTutor> tutor) {
            if (session != nullptr) {
                session->sendUnsubscription(request, tutor);
            }
        }

        /**
         * Sends a subscription change request using the current session.
         * @param request The change subscription request to send.
         * @param tutor An object that manages the reliability and retry logic for the request.
         */
        void sendSubscriptionChange(std::shared_ptr<ChangeSubscriptionRequest> request, std::shared_ptr<requests::RequestTutor> tutor) {
            if (session != nullptr) {
                session->sendSubscriptionChange(request, tutor);
            }
        }

        /**
         * Sends a reverse heartbeat request using the current session.
         * @param request The reverse heartbeat request to send.
         * @param tutor An object that manages the reliability and retry logic for the request.
         */
        void sendReverseHeartbeat(std::shared_ptr<ReverseHeartbeatRequest> request, std::shared_ptr<requests::RequestTutor> tutor) {
            if (session != nullptr) {
                session->sendReverseHeartbeat(request, tutor);
            }
        }

        /**
         * Switches to WebSocket transport.
         * @param startRecovery Indicates if recovery should start with the new session.
         */
        void switchToWebSocket(bool startRecovery) {
            createSession(false, isFrozen, false, false, false, "ip", false, false, startRecovery);
        }

        /**
         * Senses the need for switching the current session type based on the provided handler phase and cause.
         * @param handlerPhase The current phase of the handler.
         * @param switchCause The cause for potentially switching the session.
         * @param forced Indicates if the switch is forced.
         */
        void streamSense(int handlerPhase, const std::string& switchCause, bool forced) {
            if (handlerPhase != statusPhase) {
                log->warn("Mismatching phase; handler: " + std::to_string(handlerPhase) + " != " + std::to_string(statusPhase));
                return;
            }

            Status switchType = getNextSensePhase();
            log->info("Setting up new session type " + statusToString(status) + "->" + statusToString(switchType));

            if (switchType == Status::OFF || switchType == Status::END) {
                log->warn("Unexpected fallback type switching with new session");
                return;
            }

            bool strOrPoll = switchType == Status::SWITCHING_STREAMING_WS || switchType == Status::SWITCHING_STREAMING_HTTP ? STREAMING_SESSION : POLLING_SESSION;
            bool wsOrHttp = switchType == Status::SWITCHING_STREAMING_WS || switchType == Status::SWITCHING_POLLING_WS ? WS_SESSION : HTTP_SESSION;

            createSession(false, isFrozen, forced, strOrPoll, wsOrHttp, switchCause, AVOID_SWITCH, false, false);
        }

        /**
         * Prepares for a switch in session type, possibly forced, and potentially starting recovery.
         * @param handlerPhase The current phase of the handler.
         * @param switchCause The cause for the switch.
         * @param forced Indicates if the switch is forced.
         * @param startRecovery Indicates if recovery should start with the new session.
         */
        void switchReady(int handlerPhase, const std::string& switchCause, bool forced, bool startRecovery) {
            if (handlerPhase != statusPhase) {
                return;
            }

            Status switchType = status;
            log->info("Switching current session type " + statusToString(status));

            if (isNot(Status::SWITCHING_STREAMING_WS) && isNot(Status::SWITCHING_STREAMING_HTTP) && isNot(Status::SWITCHING_POLLING_HTTP) && isNot(Status::SWITCHING_POLLING_WS)) {
                log->error("Unexpected fallback type switching with a force rebind");
                return;
            }

            bool strOrPoll = switchType == Status::SWITCHING_STREAMING_WS || switchType == Status::SWITCHING_STREAMING_HTTP ? STREAMING_SESSION : POLLING_SESSION;
            bool wsOrHttp = switchType == Status::SWITCHING_STREAMING_WS || switchType == Status::SWITCHING_POLLING_WS ? WS_SESSION : HTTP_SESSION;

            bindSession(forced, strOrPoll, wsOrHttp, switchCause, startRecovery);
        }

        /**
         * Handles the transition to a slower connection mode.
         * @param handlerPhase The current phase of the handler.
         */
        void slowReady(int handlerPhase) {
            if (handlerPhase != statusPhase) {
                return;
            }
            log->info("Slow session switching");
            switchReady(handlerPhase, "slow", false, false);
        }

        /**
         * Handles session closure from the server or client.
         * @param handlerPhase The phase of the handler when the event was triggered.
         * @param noRecoveryScheduled Indicates whether a recovery was scheduled.
         * @return The current status phase after processing the closure.
         */
        int onSessionClose(int handlerPhase, bool noRecoveryScheduled) {
            if (handlerPhase != statusPhase) {
                return 0;
            }

            if (noRecoveryScheduled) {
                changeStatus(Status::OFF);
            } else {
                changeStatus(status);  // to change the statusPhase
            }

            // mpnEventManager.onSessionClose(!noRecoveryScheduled);  // Uncomment or modify as necessary

            return statusPhase;
        }

        /**
         * Senses the need to switch session type based on the current session phase and starts recovery if necessary.
         * @param handlerPhase The phase of the handler when the event was triggered.
         * @param reason The reason for switching the session type.
         * @param sessionPhase The current phase of the session.
         * @param startRecovery Indicates if recovery should start.
         */
        void streamSenseSwitch(int handlerPhase, const std::string& reason, const std::string& sessionPhase, bool startRecovery) {
            if (handlerPhase != statusPhase) {
                return;
            }

            Status switchType = getNextSensePhase();

            if (switchType == Status::OFF || switchType == Status::END) {
                log->warn("Unexpected fallback type switching with new session");
                return;
            }

            log->info("Unable to establish session of the current type. Switching session type " + statusToString(status) + "->" + statusToString(switchType));

            if (sessionPhase == "FIRST_BINDING" && status == Status::STREAMING_WS && switchType == Status::SWITCHING_STREAMING_HTTP) {
                log->debug("WebSocket support has been disabled.");
                WebSocket::disable();
            }

            changeStatus(switchType);
            startSwitchTimeout(reason, 0);
            session->requestSwitch(statusPhase, reason, false, startRecovery);
        }

        /**
         * Handles the reception of a new client IP.
         * @param clientIP The new client IP address received.
         */
        void onIPReceived(const std::string& clientIP) {
            if (!this->clientIP.empty() && clientIP != this->clientIP && WebSocket::isDisabled()) {
                WebSocket::restore();
                session->restoreWebSocket();
            }
            this->clientIP = clientIP;
        }

        /**
         * Called when a session is successfully bound to the server.
         */
        void onSessionBound() {
            // Implementation required or uncomment below if needed
            // throw std::runtime_error("Not implemented");
            nBindAfterCreate++;
        }

        /**
         * Called at the start of a session.
         */
        void onSessionStart() {
            nBindAfterCreate = 0;
        }

        /**
         * Notifies about a server-side error.
         * @param errorCode The error code received from the server.
         * @param errorMessage The error message received from the server.
         */
        void onServerError(int errorCode, const std::string& errorMessage) {
            listener->onServerError(errorCode, errorMessage);
        }

        /**
         * Responds to a slow connection detection by attempting to switch the session type accordingly.
         * @param handlerPhase The phase of the handler when the detection was made.
         * @param delay The delay before the switch should occur.
         */
        void onSlowRequired(int handlerPhase, long delay) {
            if (handlerPhase != statusPhase) {
                return;
            }

            Status switchType = getNextSlowPhase();

            log->info("Slow session detected. Switching session type " + statusToString(status) + "->" + statusToString(switchType));

            if (switchType == Status::ERROR) {
                log->error("Unexpected fallback type; switching because of a slow connection was detected " + statusToString(status) + ", " + session->toString());
                return;
            }
            changeStatus(switchType);
            startSwitchTimeout("slow", delay);
            session->requestSlow(statusPhase);
        }

        /**
         * Attempts to reconnect or re-establish a session when conditions are deemed necessary for a retry.
         * @param handlerPhase The phase of the handler at the time of retry.
         * @param retryCause The cause or reason for the retry.
         * @param forced Indicates if the retry is forced.
         * @param retryAgainIfStreamFails Indicates if another retry should be attempted if the stream fails again.
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
         * Switches to WebSocket transport.
         * @param startRecovery Indicates if recovery should start with the new session.
         */
        void switchToWebSocket(bool startRecovery) {
            createSession(false, isFrozen, false, false, false, "ip", false, false, startRecovery);
        }



    };


}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONMANAGER_HPP
