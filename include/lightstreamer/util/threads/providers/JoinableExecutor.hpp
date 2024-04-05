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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLEEXECUTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLEEXECUTOR_HPP
#include <functional>
#include <stdexcept>

namespace lightstreamer::util::threads::providers {

    /**
     * An abstract class representing a basic thread executor whose internal working
     * threads are terminated if no task arrives within a specified keep-alive time.
     */
    class JoinableExecutor {
    public:
        virtual ~JoinableExecutor() = default;

        /**
         * Executes the given task at some time in the future.
         *
         * @param task The runnable task.
         * @throws std::runtime_error if this task cannot be accepted for execution.
         * @throws std::invalid_argument if task is null.
         */
        virtual void execute(std::function<void()> task) = 0;
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLEEXECUTOR_HPP
