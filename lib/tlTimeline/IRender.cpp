// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>

#include <tlCore/FontSystem.h>

namespace tl
{
    namespace timeline
    {
        void IRender::_init(const std::shared_ptr<system::Context>& context)
        {
            _context = context;
            _fontSystem = context->getSystem<imaging::FontSystem>();
        }

        IRender::IRender()
        {}

        IRender::~IRender()
        {}
    }
}
