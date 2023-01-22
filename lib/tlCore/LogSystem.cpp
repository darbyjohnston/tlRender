// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/LogSystem.h>

#include <tlCore/Context.h>

#include <mutex>
#include <sstream>

namespace tl
{
    namespace log
    {
        bool Item::operator == (const Item& other) const
        {
            return
                time == other.time &&
                prefix == other.prefix &&
                message == other.message &&
                type == other.type;
        }

        std::string toString(const Item& item)
        {
            std::stringstream ss;
            ss.precision(2);
            switch (item.type)
            {
            case Type::Message:
                ss << std::fixed << item.time << " " << item.prefix << ": " << item.message;
                break;
            case Type::Warning:
                ss << std::fixed << item.time << " " << item.prefix << ": Warning: " << item.message;
                break;
            case Type::Error:
                ss << std::fixed << item.time << " " << item.prefix << ": ERROR: " << item.message;
                break;
            default: break;
            }
            return ss.str();
        }

        struct System::Private
        {
            std::chrono::steady_clock::time_point startTime;

            std::shared_ptr<observer::List<Item> > log;

            std::vector<Item> items;
            std::mutex mutex;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ICoreSystem::_init("tl::log:::System", context);

            TLRENDER_P();

            p.startTime = std::chrono::steady_clock::now();

            p.log = observer::List<Item>::create();
        }

        System::System() :
            _p(new Private)
        {}

        System::~System()
        {}

        std::shared_ptr<System> System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System);
                out->_init(context);
            }
            return out;
        }

        void System::print(
            const std::string& prefix,
            const std::string& value,
            Type type)
        {
            TLRENDER_P();
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> time = now - p.startTime;
            std::unique_lock<std::mutex> lock(p.mutex);
            p.items.push_back({ time.count(), prefix, value, type });
        }

        std::shared_ptr<observer::IList<Item> > System::observeLog() const
        {
            return _p->log;
        }

        void System::tick()
        {
            TLRENDER_P();
            std::vector<Item> items;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.items.swap(items);
            }
            p.log->setAlways(items);
        }

        std::chrono::milliseconds System::getTickTime() const
        {
            return std::chrono::milliseconds(100);
        }
    }
}
