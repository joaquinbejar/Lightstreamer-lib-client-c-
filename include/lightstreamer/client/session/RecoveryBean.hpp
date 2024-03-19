/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 19/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_RECOVERYBEAN_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_RECOVERYBEAN_HPP
#include <chrono>
#include <cassert>
#include "Logger.hpp"
#include <lightstreamer/client/Constants.hpp>

namespace lightstreamer::client::session {

    /**
     * @brief Bean about the status of the recovery attempt.
     *
     * State graph of the bean. The event start=T means the client wants to recover the current session.
     * Transitions not depicted should not happen.
     *
     * @verbatim
     *       start=F                            start=T
     *       +--+                               +--+
     *       |  |                               |  |
     *       |  |                               |  |
     *    +--+--v------+   start=T/set ts    +--+--v-----+
     *    |recovery=F  +--------------------->recovery=T |
     *    |            +<--------------------+           |
     *    +------------+   start=F/reset ts  +-----------+
     * @endverbatim
     */
    class RecoveryBean {
    protected:
        inline static ILogger& log = LogManager::GetLogger(Constants::SESSION_LOG);

    private:
        /**
         The flag is true when the session has been created to recover the previous session,
         which was discarded because of a network error.
         The first request sent by this session is a <seealso cref="RecoverSessionRequest"/>.
        **/
        bool recovery = false;
        std::chrono::steady_clock::time_point recoveryStartTime;
        std::chrono::steady_clock::duration recoveryDuration = std::chrono::steady_clock::duration::zero();

        bool invariant() {
            return recovery ? recoveryDuration != std::chrono::steady_clock::duration::zero() : recoveryDuration == std::chrono::steady_clock::duration::zero();
        }

    public:
        RecoveryBean() : recovery(false) {
            assert(invariant());
        }

        RecoveryBean(bool startRecovery, const RecoveryBean& old) {
            if (old.recovery) {
                if (startRecovery) {
                    recovery = true;
                    recoveryStartTime = old.recoveryStartTime;
                    recoveryDuration = old.recoveryDuration;
                } else {
                    /*
                     * This case can occur when, for example, after a recovery
                     * the client rebinds in HTTP because the opening of Websockets takes too long.
                     */
                    recovery = false;
                    recoveryDuration = std::chrono::steady_clock::duration::zero();
                }
            } else {
                if (startRecovery) {
                    recovery = true;
                    recoveryStartTime = std::chrono::steady_clock::now();
                } else {
                    recovery = false;
                    recoveryDuration = std::chrono::steady_clock::duration::zero();
                }
            }
            assert(invariant());
        }

        // Restore the time left to complete a recovery, i.e. calling timeLeftMs(maxTimeMs) returns maxTimeMs.
        // The method must be called when a recovery is successful.
        void restoreTimeLeft() {
            recovery = false;
            recoveryDuration = std::chrono::steady_clock::duration::zero();
        }

        // True when the session has been created to recover the previous session,
        // which was discarded because of a network error.
        bool Recovery() const {
            return recovery;
        }

        // Time left to recover the session.
        // When zero or a negative value, the session must be discarded.
        long timeLeftMs(long maxTimeMs) {
            if (recovery) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - recoveryStartTime).count();
                return maxTimeMs - elapsed;
            } else {
                return maxTimeMs;
            }
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_RECOVERYBEAN_HPP
