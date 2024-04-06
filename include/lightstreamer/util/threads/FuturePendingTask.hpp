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
#include <lightstreamer/util/threads/PendingTask.hpp>

namespace com::lightstreamer::util::threads {

    class FuturePendingTask : PendingTask
    {
    private:
        std::future<void> pending;
        std::future<void> token;

    public:
        FuturePendingTask(std::future<void> pending_, std::future<void> token_)
                : pending(std::move(pending_)),
                  token(std::move(token_))
        {
        }

        void Cancel()
        {
            token.get();  // Calling get() on the future representing the cancellationToken is the equivalent of calling cancel in C#.
        }

        bool IsCancellationRequested()
        {
            return pending.wait_for(std::chrono::seconds(0)) != std::future_status::timeout;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_FUTUREPENDINGTASK_HPP
