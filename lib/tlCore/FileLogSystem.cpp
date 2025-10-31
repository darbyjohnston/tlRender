// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCore/FileLogSystem.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/FileIO.h>
#include <ftk/Core/Time.h>

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

            std::shared_ptr<ftk::ListObserver<ftk::LogItem> > logObserver;

            struct Mutex
            {
                std::vector<ftk::LogItem> items;
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
            const std::shared_ptr<ftk::Context>& context,
            const std::filesystem::path& path) :
            ISystem(context, "tl::file:::FileLogSystem"),
            _p(new Private)
        {
            FTK_P();

            p.path = path;

            p.logObserver = ftk::ListObserver<ftk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<ftk::LogItem>& value)
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
                    FTK_P();
                    {
                        auto io = ftk::FileIO::create(p.path, ftk::FileMode::Write);
                    }
                    while (p.thread.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        std::vector<ftk::LogItem> items;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            std::swap(p.mutex.items, items);
                        }
                        {
                            auto io = ftk::FileIO::create(p.path, ftk::FileMode::Append);
                            for (const auto& item : items)
                            {
                                io->write(ftk::toString(item) + "\n");
                            }
                        }

                        const auto t1 = std::chrono::steady_clock::now();
                        ftk::sleep(timeout, t0, t1);
                    }
                    std::vector<ftk::LogItem> items;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        std::swap(p.mutex.items, items);
                    }
                    {
                        auto io = ftk::FileIO::create(p.path, ftk::FileMode::Append);
                        for (const auto& item : items)
                        {
                            io->write(ftk::toString(item) + "\n");
                        }
                    }
                });
        }

        FileLogSystem::~FileLogSystem()
        {
            FTK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<FileLogSystem> FileLogSystem::create(
            const std::shared_ptr<ftk::Context>& context,
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
