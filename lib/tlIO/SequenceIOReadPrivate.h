// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    namespace io
    {
        struct ISequenceRead::Private
        {
            void addTags(Info&);

            size_t threadCount = sequenceThreadCount;

            std::promise<Info> infoPromise;

            struct Request
            {
                Request() {}
                Request(Request&&) = default;

                otime::RationalTime time = time::invalidTime;
                uint16_t layer = 0;
                std::promise<VideoData> promise;

                std::string fileName;
                std::future<VideoData> future;
            };
            struct Mutex
            {
                std::list<std::shared_ptr<Request> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            struct Thread
            {
                std::list<std::shared_ptr<Request> > requestsInProgress;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
    }
}
