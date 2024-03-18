/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 18/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_RETRYDELAYCOUNTER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_RETRYDELAYCOUNTER_HPP
#include <algorithm>
#include <cmath>
#include <Logger.hpp>

namespace lightstreamer::client::session {

    /**
     * Manages retry delays, increasing them over time according to a specific policy.
     */
    class RetryDelayCounter {
    private:
        static inline Logger log = Logger::getLogger("SessionLog"); // Assumption: Logger class and getLogger method are defined elsewhere

        int attempt = 0;
        long minDelay = 0;
        long maxDelay = 0;
        long currentDelay = 0;

        void init(long delay) {
            this->currentDelay = delay;
            this->minDelay = delay;
            this->maxDelay = std::max(60000L, delay); // Ensure max delay is at least 60 seconds
            this->attempt = 0;

            if (log.isDebugEnabled()) {
                log.debug("Reset currentRetryDelay: " + std::to_string(currentDelay));
            }
        }

    public:
        explicit RetryDelayCounter(long delay) {
            init(delay);
        }

        /**
         * Resets the delay to the initial value.
         */
        void reset(long delay) {
            init(delay);
        }

        /**
         * Increases the delay, doubling it until a maximum of 60 seconds is reached.
         */
        void increase() {
            if (attempt >= 9 && currentDelay < maxDelay) {
                currentDelay *= 2;
                if (currentDelay > maxDelay) {
                    currentDelay = maxDelay;
                }

                if (log.isDebugEnabled()) {
                    log.debug("Increase currentRetryDelay: " + std::to_string(currentDelay));
                }
            }
            attempt++;
        }

        /**
         * Gets the current retry delay.
         */
        long getCurrentRetryDelay() const {
            return currentDelay;
        }

        /**
         * Gets the minimum retry delay.
         */
        long getRetryDelay() const {
            return minDelay;
        }
    };

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_RETRYDELAYCOUNTER_HPP
