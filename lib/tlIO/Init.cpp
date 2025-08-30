// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Init.h>

#include <tlIO/System.h>

#include <tlCore/Init.h>

#include <feather-tk/gl/Init.h>
#include <feather-tk/core/Context.h>

namespace tl
{
    namespace io
    {
        void init(const std::shared_ptr<ftk::Context>& context)
        {
            tl::init(context);
            ftk::gl::init(context);
            ReadSystem::create(context);
            WriteSystem::create(context);
        }
    }
}
