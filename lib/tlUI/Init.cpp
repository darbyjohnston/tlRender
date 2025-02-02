// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/Init.h>

#include <tlUI/FileBrowser.h>
#include <tlUI/MessageDialog.h>
#include <tlUI/ThumbnailSystem.h>

#include <tlIO/Init.h>

#include <dtk/core/Context.h>

namespace tl
{
    namespace ui
    {
        void init(const std::shared_ptr<dtk::Context>& context)
        {
            tl::io::init(context);
            FileBrowserSystem::create(context);
            MessageDialogSystem::create(context);
            ThumbnailSystem::create(context);
        }
    }
}
