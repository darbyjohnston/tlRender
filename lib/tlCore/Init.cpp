// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCore/Init.h>

#include <tlCore/AudioSystem.h>

#include <ftk/Core/Context.h>

namespace tl
{
    void init(const std::shared_ptr<ftk::Context>& context)
    {
        audio::System::create(context);
    }
}
