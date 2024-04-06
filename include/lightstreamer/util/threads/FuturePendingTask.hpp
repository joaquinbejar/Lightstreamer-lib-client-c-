/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 6/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_FUTUREPENDINGTASK_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_FUTUREPENDINGTASK_HPP
#include <future>
#include <memory>

namespace com::lightstreamer::util::threads {

    class PendingTask {
    public:
        virtual ~PendingTask() = default;
        virtual void Cancel() = 0;
        virtual bool IsCancellationRequested() = 0;
    };

    class FuturePendingTask : public PendingTask {
    private:
        std::shared_future<std::result_of_t<std::function<void()>()>> pending;
        std::promise<void> cancellationPromise;

    public:
        FuturePendingTask(std::future<std::result_of_t<std::function<void()>()>>&& pendingFuture)
            : pending(pendingFuture.share()) {
            // Future is initialized and shared for access
        }

        void Cancel() override {
            cancellationPromise.set_value(); // Signal cancellation
        }

        bool IsCancellationRequested() override {
            return pending.wait_for(std::chrono::seconds(0)) == std::future_status::deferred;
            // This checks if the task is waiting to start or has been deferred, which might indicate cancellation.
            // Note: This does not directly check for cancellation but rather infers it based on the task state.
        }

        // Additional methods to work with the future and cancellation state as needed
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_FUTUREPENDINGTASK_HPP
