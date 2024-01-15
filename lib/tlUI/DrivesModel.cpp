// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/DrivesModel.h>

#include <tlCore/Path.h>
#include <tlCore/Time.h>
#include <tlCore/Timer.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace ui
    {
        namespace
        {
            const std::chrono::milliseconds timeout(100);
        }

        struct DrivesModel::Private
        {
            std::shared_ptr<observer::List<std::string> > drives;

            struct Mutex
            {
                std::vector<std::string> drives;
                std::mutex mutex;
            };
            Mutex mutex;
            std::thread thread;
            std::atomic<bool> running;

            std::shared_ptr<time::Timer> timer;
        };

        void DrivesModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.drives = observer::List<std::string>::create();

            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    while (p.running)
                    {
                        const std::vector<std::string> drives = file::getDrives();
                        {
                            std::lock_guard<std::mutex> lock(p.mutex.mutex);
                            p.mutex.drives = drives;
                        }
                        time::sleep(timeout);
                    }
                });

            p.timer = time::Timer::create(context);
            p.timer->setRepeating(true);
            p.timer->start(
                timeout,
                [this]
                {
                    TLRENDER_P();
                    std::vector<std::string> drives;
                    {
                        std::lock_guard<std::mutex> lock(p.mutex.mutex);
                        drives = p.mutex.drives;
                    }
                    p.drives->setIfChanged(drives);
                });
        }

        DrivesModel::DrivesModel() :
            _p(new Private)
        {}

        DrivesModel::~DrivesModel()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<DrivesModel> DrivesModel::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<DrivesModel>(new DrivesModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<std::string> > DrivesModel::observeDrives() const
        {
            return _p->drives;
        }
    }
}