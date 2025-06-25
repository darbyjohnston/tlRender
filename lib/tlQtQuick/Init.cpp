// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtQuick/Init.h>

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlQt/Init.h>

#include <feather-tk/core/Context.h>

namespace tl
{
    namespace qtquick
    {
        namespace
        {
            std::shared_ptr<feather_tk::Context> _context;
        }

        void init(
            const std::shared_ptr<feather_tk::Context>& context,
            qt::DefaultSurfaceFormat defaultSurfaceFormat)
        {
            qt::init(context, defaultSurfaceFormat);
            System::create(context);
         }

        System::System(const std::shared_ptr<feather_tk::Context>& context) :
            ISystem(context, "tl::qtquick::System")
        {
            _context = context;

            qmlRegisterType<GLFramebufferObject>("tlQtQuick", 1, 0, "GLFramebufferObject");
        }

        System::~System()
        {
            _context.reset();
        }

        std::shared_ptr<System> System::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System(context));
                context->addSystem(out);
            }
            return out;
        }

        const std::shared_ptr<feather_tk::Context>& getContext()
        {
            return _context;
        }
    }
}
