// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/Init.h>

#include <tlIO/System.h>

#include <tlCore/Init.h>

#include <ftk/GL/Init.h>
#include <ftk/Core/Context.h>

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
