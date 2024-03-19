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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_OFFLINECHECK_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_OFFLINECHECK_HPP
#include <chrono>
#include <mutex>
#include <thread>
#include "Logger.hpp"
#include <lightstreamer/client/Constants.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>


namespace lightstreamer::client::session {

    class OfflineCheck {
    private:
        static constexpr int MAYBE_ONLINE_TIMEOUT = 20000; // 20 segundos
        static constexpr int OFFLINE_CHECKS_PROTECTION = 1;
        static constexpr long OFFLINE_TIMEOUT = 1000; // 1 segundo

        std::atomic<int> maybeOnline{1};
        std::atomic<int> maybePhase{1};
        SessionThread thread;

        static inline ILogger& log = LogManager::GetLogger(Constants::TRANSPORT_LOG);

    public:
        explicit OfflineCheck(/* SessionThread thread */) {
            this->thread = thread; // TODO: adapt to your thread control implementation
        }

        bool shouldDelay(const std::string& server) {
            if (OfflineStatus::isOffline(server)) { // TODO: OfflineStatus::isOffline

                log.Debug("Offline check: " + std::to_string(maybeOnline.load()));

                if (maybeOnline <= 0) {
                    return true;
                } else {
                    maybeOnline--;

                    if (maybeOnline == 0) {
                        int ph = maybePhase.load();

                        log.Debug("Offline check 0.");

                        std::thread([this, ph]() {
                            std::this_thread::sleep_for(std::chrono::milliseconds(MAYBE_ONLINE_TIMEOUT));
                            resetMaybeOnline(ph);
                        }).detach(); // TODO: check if detach is the best option
                    }
                }
            }
            return false;
        }

        void resetMaybeOnline() {
            resetMaybeOnline(maybePhase);
        }

    private:
        void resetMaybeOnline(int mp) {
            if (mp != maybePhase) {
                return;
            }

            log.Debug("Offline check 1.");

            maybePhase++;
            maybeOnline = OFFLINE_CHECKS_PROTECTION;
        }

    public:
        long getDelay() const {
            return OFFLINE_TIMEOUT;
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_OFFLINECHECK_HPP
