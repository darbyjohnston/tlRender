// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Init.h>

#include <tlUI/FileBrowser.h>
#include <tlUI/MessageDialog.h>

#include <tlIO/Init.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace ui
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            tl::io::init(context);
            if (!context->getSystem<FileBrowserSystem>())
            {
                context->addSystem(FileBrowserSystem::create(context));
            }
            if (!context->getSystem<MessageDialogSystem>())
            {
                context->addSystem(MessageDialogSystem::create(context));
            }
        }
    }
}
