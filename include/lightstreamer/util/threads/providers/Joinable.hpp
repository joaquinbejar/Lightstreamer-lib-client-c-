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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLE_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLE_HPP

namespace lightstreamer::util::threads::providers {

    /**
     * Root abstract class for joinable executors and schedulers.
     *
     * Executors and Schedulers are said to be joinable if their internal working threads are
     * terminated when no more tasks arrive, therefore allowing a graceful completion of involved
     * threads without the need to explicitly invoke a shutdown method.
     */
    class Joinable {
    public:
        virtual ~Joinable() = default;

        /**
         * Waits indefinitely for this joinable executor (or scheduler) to complete all tasks.
         *
         * @throws std::runtime_error which wraps an std::exception if the current thread is interrupted.
         */
        virtual void join() = 0;
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLE_HPP
