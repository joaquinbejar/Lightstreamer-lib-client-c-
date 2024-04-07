/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 7/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_THREADMULTIPLEXER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_THREADMULTIPLEXER_HPP
#include <functional>
#include <future>
#include <memory>

namespace com::lightstreamer::util::threads {

    template<typename S>
    class ThreadMultiplexer {
    public:
        virtual ~ThreadMultiplexer() = default;

        /**
         * Executes the given task.
         *
         * @param source The source object associated with the task.
         * @param runnable The task to execute, represented as a std::function<void()>.
         */
        virtual void execute(S source, std::function<void()> runnable) = 0;

        /**
         * Schedules the given task to be executed after a delay.
         *
         * @param source The source object associated with the task.
         * @param task The task to execute, represented as a std::function<void()>.
         * @param delayMillis The delay in milliseconds before the task is executed.
         * @return A shared_ptr to a std::promise<void> that can be used to signal cancellation.
         */
        virtual std::shared_ptr<std::promise<void>> schedule(S source, std::function<void()> task, long delayMillis) = 0;

        /**
         * Waits for all scheduled tasks to complete.
         */
        virtual void await() = 0;
    };

}


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_THREADMULTIPLEXER_HPP
