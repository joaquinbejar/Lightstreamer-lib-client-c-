/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 5/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLESCHEDULER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLESCHEDULER_HPP
#include <functional>
#include <future>
#include <chrono>
#include <stdexcept>
#include <lightstreamer/util/threads/providers/Joinable.hpp>

namespace lightstreamer::util::threads::providers {

    /**
     * An abstract class representing a basic thread scheduler whose internal working threads are terminated if
     * no task arrives within a specified keep-alive time.
     */
    class JoinableScheduler : Joinable {
    public:
        virtual ~JoinableScheduler() = default;

        /**
         * Creates and executes a one-shot action that becomes enabled after the given delay.
         *
         * @param task The task to execute.
         * @param delayInMillis The time in milliseconds from now to delay execution.
         * @return A std::future object representing pending completion of the task.
         * @throws std::invalid_argument if the task is null.
         * @throws std::runtime_error if the task cannot be scheduled for execution.
         */
        virtual std::shared_ptr<std::future<void>> schedule(std::function<void()> task, long delayInMillis) = 0;
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLESCHEDULER_HPP
