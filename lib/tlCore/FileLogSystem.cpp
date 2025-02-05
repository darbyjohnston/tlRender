// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/FileLogSystem.h>

#include <dtk/core/Context.h>
#include <dtk/core/FileIO.h>
#include <dtk/core/Time.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace file
    {
        namespace
        {
            const std::chrono::milliseconds timeout(1000);
        }

        struct FileLogSystem::Private
        {
            std::string fileName;

            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;

            struct Mutex
            {
                std::vector<dtk::LogItem> items;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        FileLogSystem::FileLogSystem(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& fileName) :
            ISystem(context, "tl::file:::FileLogSystem"),
            _p(new Private)
        {
            DTK_P();

            p.fileName = fileName;

            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    std::unique_lock<std::mutex> lock(_p->mutex.mutex);
                    _p->mutex.items.insert(
                        _p->mutex.items.end(),
                        value.begin(),
                        value.end());
                });
            
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    DTK_P();
                    {
                        auto io = dtk::FileIO::create(p.fileName, dtk::FileMode::Write);
                    }
                    while (p.thread.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        std::vector<dtk::LogItem> items;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            std::swap(p.mutex.items, items);
                        }
                        {
                            auto io = dtk::FileIO::create(p.fileName, dtk::FileMode::Append);
                            io->seek(io->getSize());
                            for (const auto& item : items)
                            {
                                io->write(dtk::toString(item) + "\n");
                            }
                        }

                        const auto t1 = std::chrono::steady_clock::now();
                        dtk::sleep(timeout, t0, t1);
                    }
                    std::vector<dtk::LogItem> items;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        std::swap(p.mutex.items, items);
                    }
                    {
                        auto io = dtk::FileIO::create(p.fileName, dtk::FileMode::Append);
                        io->seek(io->getSize());
                        for (const auto& item : items)
                        {
                            io->write(dtk::toString(item) + "\n");
                        }
                    }
                });
        }

        FileLogSystem::~FileLogSystem()
        {
            DTK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<FileLogSystem> FileLogSystem::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& fileName)
        {
            auto out = context->getSystem<FileLogSystem>();
            if (!out)
            {
                out = std::shared_ptr<FileLogSystem>(new FileLogSystem(context, fileName));
                context->addSystem(out);
            }
            return out;
        }
    }
}
