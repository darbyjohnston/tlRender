// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/Init.h>

#include <tlTimeline/MemoryReference.h>

#include <tlIO/Init.h>

#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>

#include <opentimelineio/typeRegistry.h>

namespace tl
{
    namespace timeline
    {
        void init(const std::shared_ptr<ftk::Context>& context)
        {
            io::init(context);
            System::create(context);
        }

        System::System(const std::shared_ptr<ftk::Context>& context) :
            ISystem(context, "tl::timeline::System")
        {
            const std::vector<std::pair<std::string, bool> > registerTypes
            {
                {
                    "RawMemoryReference",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::RawMemoryReference>()
                },
                {
                    "SharedMemoryReference",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::SharedMemoryReference>()
                },
                {
                    "RawMemorySequenceReference",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::RawMemorySequenceReference>()
                },
                {
                    "SharedMemorySequenceReference",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::SharedMemorySequenceReference>()
                },
                {
                    "ZipMemoryReference",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::ZipMemoryReference>()
                },
                {
                    "ZipMemorySequenceReference",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::ZipMemorySequenceReference>()
                }
            };
            for (const auto& t : registerTypes)
            {
                _log(ftk::Format("Register type {0}: {1}").
                    arg(t.first).
                    arg(t.second));
            }
        }

        System::~System()
        {}

        std::shared_ptr<System> System::create(const std::shared_ptr<ftk::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System(context));
                context->addSystem(out);
            }
            return out;
        }
    }
}
