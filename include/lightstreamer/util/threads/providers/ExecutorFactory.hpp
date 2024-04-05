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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_EXECUTORFACTORY_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_EXECUTORFACTORY_HPP
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <lightstreamer/util/threads/providers/JoinableScheduler.hpp>
#include <lightstreamer/util/threads/providers/JoinableExecutor.hpp>

namespace lightstreamer::util::threads::providers {


    /**
     * A Factory of joinable Executors or Schedulers.
     *
     * The entry point of the factory is the getDefaultExecutorFactory() static method,
     * which provides an instance of the class DefaultExecutorFactory. To provide a custom
     * implementation, it is required to pass it to the setDefaultExecutorFactory(ExecutorFactory),
     * before the library is actually used.
     */
    class ExecutorFactory {
    private:
        static std::shared_ptr<ExecutorFactory> defaultExecutorFactory;
        static std::mutex mutex;

    public:
        static std::shared_ptr<ExecutorFactory>& getDefaultExecutorFactory() {
            std::lock_guard<std::mutex> lock(mutex);
            if (!defaultExecutorFactory) {
                // Placeholder for alternative loading mechanism
                // If loading fails, instantiate DefaultExecutorFactory or a custom implementation
                defaultExecutorFactory = std::make_shared<ExecutorFactory>();
            }
            return defaultExecutorFactory;
        }

        static void setDefaultExecutorFactory(std::shared_ptr<ExecutorFactory> factory) {
            if (!factory) {
                throw std::invalid_argument("Specify a factory");
            }
            std::lock_guard<std::mutex> lock(mutex);
            defaultExecutorFactory = factory;
        }

        /**
         * Configure and returns a new JoinableExecutor instance, as per the specified parameters.
         *
         * @param nThreads The number of threads of the thread pool.
         * @param threadName The suffix to use for the name of every newly created thread.
         * @param keepAliveTime The keep-alive time specified in milliseconds.
         * @return A new instance of JoinableExecutor.
         */
        virtual std::shared_ptr<JoinableExecutor> getExecutor(int nThreads, const std::string& threadName, long keepAliveTime) {
            // Placeholder for actual JoinableExecutor creation
            return nullptr; // Implement according to your needs
        }

        /**
         * Configure and returns a new JoinableScheduler instance, as per the specified parameters.
         *
         * @param nThreads The number of threads of the thread pool.
         * @param threadName The suffix to use for the name of every newly created thread.
         * @param keepAliveTime The keep-alive time specified in milliseconds.
         * @return A new instance of JoinableScheduler.
         */
        virtual std::shared_ptr<JoinableScheduler> getScheduledExecutor(int nThreads, const std::string& threadName, long keepAliveTime) {
            // Placeholder for actual JoinableScheduler creation
            return nullptr; // Implement according to your needs
        }

        virtual std::shared_ptr<JoinableScheduler> getScheduledExecutor(int nThreads, const std::string& threadName, long keepAliveTime, std::shared_ptr<JoinableExecutor> executor) {
            // Placeholder for actual JoinableScheduler creation
            return nullptr; // Implement according to your needs
        }
    };

    // Static member initialization
    std::shared_ptr<ExecutorFactory> ExecutorFactory::defaultExecutorFactory = nullptr;
    std::mutex ExecutorFactory::mutex;
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_EXECUTORFACTORY_HPP
