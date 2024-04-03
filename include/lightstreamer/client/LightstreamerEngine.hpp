/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 3/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LIGHTSTREAMERENGINE_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LIGHTSTREAMERENGINE_HPP
#include "lightstreamer/client/events/EventsThread.hpp"
#include "lightstreamer/client/session/InternalConnectionOptions.hpp"
#include "lightstreamer/client/session/SessionManager.hpp"
#include "lightstreamer/client/session/SessionsListener.hpp"
#include "lightstreamer/client/session/SessionThread.hpp"
#include "lightstreamer/client/transport/WebSocket.hpp"


namespace lightstreamer::client {
    class LightstreamerEngine {
    private:
        static const bool FROM_API = true;
        static const bool NO_TRANSPORT_FORCED = false;
        static const bool NO_COMBO_FORCED = false;
        static const bool NO_POLLING = false;
        static const bool CAN_SWITCH = false;
        static const bool NO_RECOVERY = true;

        ILogger *log = LogManager::GetLogger(Constants::SESSION_LOG);

        session::SessionManager *sessionManager;
        session::InternalConnectionOptions *connectionOptions;
        session::SessionThread *sessionThread;
        EventsThread *eventsThread;
        ClientListener *clientListener;

        bool connectionRequested = false;

    public:
        LightstreamerEngine(session::InternalConnectionOptions *options, session::SessionThread *sessionThread,
                             EventsThread *eventsThread, ClientListener *listener, session::SessionManager *manager) {
            this->connectionOptions = options;
            this->sessionThread = sessionThread;
            this->clientListener = listener;
            this->eventsThread = eventsThread;

            this->sessionManager = manager;
            manager->SessionsListener = new SessionsListenerImpl(this);
        }

        void connect() {
            this->connect(false);
        }

        void connect(bool forced) {
            this->connectionRequested = true;

            sessionThread->queue([&]() {
                std::string currentStatus = sessionManager->getHighLevelStatus(false);
                if (!forced && (currentStatus == Constants::CONNECTING || currentStatus == Constants::STALLED ||
                                currentStatus.rfind(Constants::CONNECTED, 0) == 0)) {
                    return;
                }

                std::string ft = connectionOptions->ForcedTransport;

                if (ft.empty()) {
                    bool isHttp = transport::WebSocket::Disabled;
                    sessionManager->createSession(FROM_API, NO_TRANSPORT_FORCED, NO_COMBO_FORCED, NO_POLLING, isHttp,
                                                  nullptr, CAN_SWITCH, false, false);
                } else {
                    bool isPolling = ft == Constants::WS_POLLING || ft == Constants::HTTP_POLLING;
                    bool isHTTP = ft == Constants::HTTP_POLLING || ft == Constants::HTTP_STREAMING ||
                                  ft == Constants::HTTP_ALL;
                    bool isTransportForced = ft == Constants::WS_ALL || ft == Constants::HTTP_ALL;
                    bool isComboForced = !isTransportForced;

                    sessionManager->createSession(FROM_API, isTransportForced, isComboForced, isPolling, isHTTP,
                                                  nullptr, CAN_SWITCH, false, false);
                }
            });
        }

        void disconnect() {
            this->connectionRequested = false;

            sessionThread->queue([&]() {
                log->Debug("Closing a new session and stopping automatic reconnections");

                sessionManager->closeSession(FROM_API, "api", NO_RECOVERY);
            });
        }

        // other methods ...

    private:
        class SessionsListenerImpl : public SessionsListener {
        private:
            LightstreamerEngine *outerInstance;

        public:
            SessionsListenerImpl(LightstreamerEngine *outerInstance) {
                this->outerInstance = outerInstance;
            }

            void onStatusChanged(std::string status) override {
                outerInstance->eventsThread->queue([=]() {
                    outerInstance->clientListener->onStatusChange(status);
                });
            }

            void onServerError(int errorCode, std::string errorMessage) override {
                outerInstance->eventsThread->queue([=]() {
                    outerInstance->clientListener->onServerError(errorCode, errorMessage);
                });
            }
        };
    };
}
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LIGHTSTREAMERENGINE_HPP
