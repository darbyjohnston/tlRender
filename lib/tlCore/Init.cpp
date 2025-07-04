// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/Init.h>

#include <tlCore/AudioSystem.h>

#include <feather-tk/core/Context.h>

namespace tl
{
    void init(const std::shared_ptr<feather_tk::Context>& context)
    {
        audio::System::create(context);
    }
}
