// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/FileLogSystem.h>

#include <feather-tk/core/Context.h>
#include <feather-tk/core/FileIO.h>
#include <feather-tk/core/Time.h>

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
            std::filesystem::path path;

            std::shared_ptr<feather_tk::ListObserver<feather_tk::LogItem> > logObserver;

            struct Mutex
            {
                std::vector<feather_tk::LogItem> items;
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
            const std::shared_ptr<feather_tk::Context>& context,
            const std::filesystem::path& path) :
            ISystem(context, "tl::file:::FileLogSystem"),
            _p(new Private)
        {
            FEATHER_TK_P();

            p.path = path;

            p.logObserver = feather_tk::ListObserver<feather_tk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<feather_tk::LogItem>& value)
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
                    FEATHER_TK_P();
                    {
                        auto io = feather_tk::FileIO::create(p.path, feather_tk::FileMode::Write);
                    }
                    while (p.thread.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        std::vector<feather_tk::LogItem> items;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            std::swap(p.mutex.items, items);
                        }
                        {
                            auto io = feather_tk::FileIO::create(p.path, feather_tk::FileMode::Append);
                            for (const auto& item : items)
                            {
                                io->write(feather_tk::toString(item) + "\n");
                            }
                        }

                        const auto t1 = std::chrono::steady_clock::now();
                        feather_tk::sleep(timeout, t0, t1);
                    }
                    std::vector<feather_tk::LogItem> items;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        std::swap(p.mutex.items, items);
                    }
                    {
                        auto io = feather_tk::FileIO::create(p.path, feather_tk::FileMode::Append);
                        for (const auto& item : items)
                        {
                            io->write(feather_tk::toString(item) + "\n");
                        }
                    }
                });
        }

        FileLogSystem::~FileLogSystem()
        {
            FEATHER_TK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<FileLogSystem> FileLogSystem::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::filesystem::path& path)
        {
            auto out = context->getSystem<FileLogSystem>();
            if (!out)
            {
                out = std::shared_ptr<FileLogSystem>(new FileLogSystem(context, path));
                context->addSystem(out);
            }
            return out;
        }
    }
}
