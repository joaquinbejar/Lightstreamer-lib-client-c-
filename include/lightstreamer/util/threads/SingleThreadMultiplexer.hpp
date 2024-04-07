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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SINGLETHREADMULTIPLEXER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SINGLETHREADMULTIPLEXER_HPP
#include <memory>
#include <functional>
#include <future>
#include <chrono>
#include <iostream>
#include <lightstreamer/util/threads/providers/JoinableExecutor.hpp>
#include <lightstreamer/util/threads/providers/JoinableScheduler.hpp>
#include <lightstreamer/util/threads/providers/ExecutorFactory.hpp>

namespace lightstreamer::util::threads {

    template<typename S>
    class SingleThreadMultiplexer : public ThreadMultiplexer<S> {
    private:
        std::shared_ptr<providers::JoinableExecutor> executor;
        std::shared_ptr<providers::JoinableScheduler> scheduler;

    public:
        SingleThreadMultiplexer() {
            // Assuming ExecutorFactory provides shared_ptr to executors and schedulers
            executor = providers::ExecutorFactory::getDefaultExecutorFactory()->getExecutor(1, "Session Thread", 1000);
            scheduler = providers::ExecutorFactory::getDefaultExecutorFactory()->getScheduledExecutor(1, "Session Thread", 1000, executor);
        }

        void await()  {
            std::cout << "Await executor ... " << std::endl;
            executor->join();
            std::cout << "Await scheduler ... " << std::endl;
            scheduler->join();
            std::cout << "Await done." << std::endl;
        }

        void execute(S source, std::function<void()> runnable) override {
            executor->execute(runnable);
            // Alternatively, you can schedule immediately with scheduler, depending on use case
        }

        std::shared_ptr<std::future<void>> schedule(S source, std::function<void()> task, long delayMillis) override {
            // Implementation depends on how JoinableScheduler's schedule method is defined.
            // Here's a simple placeholder assuming schedule returns a future.
            return scheduler->schedule(task, delayMillis);
        }
    };

    // ExecutorFactory definition needs to be provided
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SINGLETHREADMULTIPLEXER_HPP
