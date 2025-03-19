// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Init.h>

#include <tlIO/System.h>

#include <tlCore/Init.h>

#include <dtk/gl/Init.h>
#include <dtk/core/Context.h>

namespace tl
{
    namespace io
    {
        void init(const std::shared_ptr<dtk::Context>& context)
        {
            tl::init(context);
            dtk::gl::init(context);
            ReadSystem::create(context);
            WriteSystem::create(context);
        }
    }
}
