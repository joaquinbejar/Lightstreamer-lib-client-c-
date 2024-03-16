/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REVERSEHEARTBEATTIMER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REVERSEHEARTBEATTIMER_HPP
#include <chrono>
#include <cassert>
#include <cmath>
#include <functional>
#include <thread>
#include <mutex>
#include "SessionThread.h" // Placeholder for actual SessionThread implementation
#include "InternalConnectionOptions.h" // Placeholder for actual options class
#include "ReverseHeartbeatRequest.h" // Placeholder for the actual request class
#include "VoidTutor.h" // Placeholder for actual tutor class
#include "Logger.h" // Placeholder for actual logging

namespace lightstreamer::client::protocol {

    class ReverseHeartbeatTimer {
    private:
        static Logger log;

        SessionThread& sessionThread;
        InternalConnectionOptions& options;
        const long maxIntervalMs;
        long currentIntervalMs = -1;
        bool disableHeartbeats = false;
        bool closed = false;
        std::chrono::steady_clock::time_point lastSentTime = std::chrono::steady_clock::now();
        int currentPhase = 0;
        bool bindSent = false;

    public:
        ReverseHeartbeatTimer(SessionThread& sessionThread, InternalConnectionOptions& options)
                : sessionThread(sessionThread), options(options), maxIntervalMs(options.getReverseHeartbeatInterval()) {
            log.debug("rhb max interval " + std::to_string(maxIntervalMs));
            setCurrentInterval(maxIntervalMs);
        }

        void onBindSession(bool bindAsControl) {
            if (bindAsControl) {
                lastSentTime = std::chrono::steady_clock::now();
            }
            if (!bindSent) {
                bindSent = true;
                schedule();
            }
        }

        void onChangeInterval() {
            long newInterval = options.getReverseHeartbeatInterval();
            setCurrentInterval(newInterval);
        }

        void onControlRequest() {
            lastSentTime = std::chrono::steady_clock::now();
        }

        void onClose() {
            closed = true;
        }

        long getMaxIntervalMs() const {
            return maxIntervalMs;
        }

    private:
        void schedule() {
            if (disableHeartbeats || closed) {
                return;
            }

            auto timeLeftMs = getTimeLeftMs();
            if (timeLeftMs <= 0) {
                sendHeartbeat();
                submitTask(currentIntervalMs);
            } else {
                submitTask(timeLeftMs);
            }
        }

        long getTimeLeftMs() const {
            assert(lastSentTime.time_since_epoch().count() != -1);
            assert(currentIntervalMs != -1);
            auto now = std::chrono::steady_clock::now();
            auto timeElapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSentTime).count();
            long timeLeftMs = currentIntervalMs - timeElapsedMs;
            return timeLeftMs;
        }

        // Assuming the existence and inclusion of relevant headers for ReverseHeartbeatRequest, VoidTutor, and SessionThread
        void sendHeartbeat() {
            // Create instances of ReverseHeartbeatRequest and VoidTutor
            auto request = std::make_shared<ReverseHeartbeatRequest>();
            auto tutor = std::make_shared<VoidTutor>(sessionThread, options);

            // Assuming sessionThread has a way to access a SessionManager that has the sendReverseHeartbeat method
            sessionThread.getSessionManager()->sendReverseHeartbeat(request, tutor);
        }


        /*
         * Change the current interval with respect to the user defined value and the maximum interval.
         *
         * newInterval      currentIntervalMs   maxIntervalMs   new currentIntervalMs
         * --------------------------------------------------------------------------------------------------
         * ∞                ∞                   ∞               ∞
         * ∞                ∞                   m               impossible: currentIntervalMs > maxIntervalMs
         * ∞                c                   ∞               ∞
         * ∞                c                   m               m
         * u                ∞                   ∞               u
         * u                ∞                   m               impossible: currentIntervalMs > maxIntervalMs
         * u                c                   ∞               u
         * u                c                   m               minimum(u, m)
         *
         * ∞ = interval is 0
         * u, c, m = interval bigger than 0
         */
        void setCurrentInterval(long interval) {
            assert(maxIntervalMs != -1);
            long oldIntervalMs = currentIntervalMs;
            if (interval == 0) {
                currentIntervalMs = maxIntervalMs;
            } else if (maxIntervalMs == 0) {
                assert(interval > 0);
                currentIntervalMs = interval;
            } else {
                assert(interval > 0 && maxIntervalMs > 0);
                currentIntervalMs = std::min(interval, maxIntervalMs);
            }
            disableHeartbeats = (currentIntervalMs == 0);
            if (oldIntervalMs != currentIntervalMs) {
                log.debug("rhb current interval " + std::to_string(currentIntervalMs));
                if (bindSent) {
                    currentPhase++;
                    schedule();
                }
            }
        }

    private:
        void submitTask(long scheduleTimeMs) {
            if (log.isDebugEnabled()) {
                log.debug("rhb scheduled +" + std::to_string(scheduleTimeMs) + " ph " + std::to_string(currentPhase));
            }

            int scheduledPhase = currentPhase;
            auto task = [this, scheduledPhase]() {
                if (log.isDebugEnabled()) {
                    log.debug("rhb task fired ph " + std::to_string(scheduledPhase));
                }
                if (scheduledPhase < currentPhase) {
                    if (log.isDebugEnabled()) {
                        log.debug("rhb task discarded ph " + std::to_string(scheduledPhase));
                    }
                    return;
                }
                assert(scheduledPhase == currentPhase);
                this->schedule();
            };

            std::thread([task, scheduleTimeMs]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(scheduleTimeMs));
                task();
            }).detach(); // Detach the thread to let it run independently
        }
    };

// Initialization of the static logger
    Logger ReverseHeartbeatTimer::log = LogManager::getLogger(Constants::HEARTBEAT_LOG);

} // namespace com::lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REVERSEHEARTBEATTIMER_HPP
