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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_DEFAULTEXECUTORFACTORY_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_DEFAULTEXECUTORFACTORY_HPP
#include <lightstreamer/util/threads/providers/ExecutorFactory.hpp>
#include <lightstreamer/util/threads/providers/JoinableScheduler.hpp>
#include <lightstreamer/util/threads/providers/JoinableExecutor.hpp>



namespace lightstreamer::util::threads {

    /**
     * The default implementation of an ExecutorFactory.
     */
    class DefaultExecutorFactory : public providers::ExecutorFactory {
    public:
        /**
         * Returns a new instance of a JoinableExecutor.
         *
         * @param nThreads Number of threads in the executor pool.
         * @param threadName Name prefix for threads in the pool.
         * @param keepAliveTime Time in milliseconds that threads without tasks will wait for new tasks before terminating.
         * @return A shared pointer to a JoinableExecutor instance.
         */
        std::shared_ptr<providers::JoinableExecutor> getExecutor(int nThreads, const std::string& threadName, long keepAliveTime) override {
            // Placeholder for actual CSJoinableExecutor creation
            return std::make_shared<CSJoinableExecutor>(threadName, keepAliveTime);
        }

        /**
         * Returns a new instance of a JoinableScheduler.
         *
         * @param nThreads Number of threads in the scheduler pool.
         * @param threadName Name prefix for threads in the pool.
         * @param keepAliveTime Time in milliseconds that threads without tasks will wait for new tasks before terminating.
         * @return A shared pointer to a JoinableScheduler instance.
         */
        std::shared_ptr<providers::JoinableScheduler> getScheduledExecutor(int nThreads, const std::string& threadName, long keepAliveTime) override {
            // Placeholder for actual CSJoinableScheduler creation
            return std::make_shared<CSJoinableScheduler>(threadName, keepAliveTime);
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_DEFAULTEXECUTORFACTORY_HPP
