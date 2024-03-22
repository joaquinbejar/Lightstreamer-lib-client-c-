/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 22/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SLOWINGHANDLER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SLOWINGHANDLER_HPP
#include <chrono>
#include <cmath>
#include "Logger.hpp"
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>
#include <lightstreamer/client/Constants.hpp>


namespace lightstreamer::client::session {

    class SlowingHandler {
        static constexpr double MOMENTUM = 0.5;
        static constexpr double MAX_MEAN = 7000;
        static constexpr double HUGE_DELAY = 20000;
        static constexpr double IGNORE_MEAN = 60;

        ILogger& log = LogManager::GetLogger(Constants::SESSION_LOG);

        std::chrono::steady_clock::time_point refTime;
        double meanElaborationDelay = 0;
        bool firstMeanCalculated = false;
        bool hugeFlag = false;
        InternalConnectionOptions& options;

    public:
        explicit SlowingHandler(InternalConnectionOptions& options) : options(options) {}

        long getDelay() const {
            if (!firstMeanCalculated || meanElaborationDelay <= 0) {
                return 0;
            } else {
                return static_cast<long>(std::round(std::floor(meanElaborationDelay)));
            }
        }

        double getMeanElaborationDelay() const {
            return meanElaborationDelay;
        }

        void setMeanElaborationDelay(double value) {
            firstMeanCalculated = true;
            meanElaborationDelay = value;
        }

        void startSync(bool isStreaming, bool forced, double currTime) {
            if (isStreaming || forced) {
                meanElaborationDelay = 0;
                hugeFlag = false;
            }
            refTime = std::chrono::steady_clock::now();
        }

        void testPollSync(long millis, double currTime) {
            testSync(millis, currTime);
        }

        bool syncCheck(long seconds, bool isStreaming, double currTime) {
            if (isStreaming) {
                log.Debug("Sync check: " + std::to_string(seconds) + " - " + std::to_string(currTime));
                bool syncProblem = testSync(seconds * 1000, currTime);
                if (!syncProblem) {
                    return true;
                } else if (options.isSlowingEnabled()) {
                    log.Info("Slow connection detected");
                    return false;
                }
            } else {
                log.Warn("Unexpected synchronization call during polling session");
            }
            return true;
        }

    private:
        bool testSync(long millis, double currTime) {
            auto now = std::chrono::steady_clock::now();
            auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - refTime).count() - millis;
            if (!firstMeanCalculated) {
                setMeanElaborationDelay(diffTime);
                return false;
            } else {
                if (diffTime > HUGE_DELAY && diffTime > meanElaborationDelay * 2) {
                    hugeFlag = !hugeFlag;
                    if (hugeFlag) {
                        log.Info("Huge delay detected by sync signals. Restored from standby/hibernation?");
                        return meanElaborationDelay > MAX_MEAN;
                    }
                }
                setMeanElaborationDelay(meanElaborationDelay * MOMENTUM + diffTime * (1 - MOMENTUM));
                if (meanElaborationDelay < IGNORE_MEAN) {
                    setMeanElaborationDelay(0);
                    return false;
                } else if (meanElaborationDelay > MAX_MEAN) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SLOWINGHANDLER_HPP
