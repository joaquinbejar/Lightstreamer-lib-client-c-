/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 28/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_IDGENERATOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_IDGENERATOR_HPP
#include <atomic>

namespace lightstreamer::util {

    /**
     * Class responsible for generating unique request and subscription IDs.
     */
    class IdGenerator {
    private:
        static std::atomic<long> requestIdGenerator;
        static std::atomic<int> subscriptionIdGenerator;

    public:
        /**
         * Generates the next request ID to be used as the value of the parameter LS_reqId.
         * @return The next request ID.
         */
        static long NextRequestId() {
            return ++requestIdGenerator;
        }

        /**
         * Generates the next subscription ID to be used as the value of the parameter LS_subId.
         * @return The next subscription ID.
         */
        static int NextSubscriptionId() {
            return ++subscriptionIdGenerator;
        }
    };

    // Initialize the static atomic members.
    std::atomic<long> IdGenerator::requestIdGenerator{0};
    std::atomic<int> IdGenerator::subscriptionIdGenerator{0};

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_IDGENERATOR_HPP
