// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtQuick/Util.h>

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlQt/Util.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace qtquick
    {
        namespace
        {
            std::weak_ptr<system::Context> _context;
        }

        void init(const std::shared_ptr<system::Context>& context)
        {
            qt::init(context);

            _context = context;

            qmlRegisterType<GLFramebufferObject>("tlQtQuick", 1, 0, "GLFramebufferObject");

            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }

        const std::weak_ptr<system::Context>& context()
        {
            return _context;
        }
    }
}
