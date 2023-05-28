// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>

#include <QColor>
#include <QFont>

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! Qt QWidget support.
    namespace qtwidget
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(const std::shared_ptr<system::Context>&);

        //! Initialize the fonts. This needs to be called after the Qt
        //! application is created.
        void initFonts(const std::shared_ptr<system::Context>& context);

        //! Convert a Qt color.
        imaging::Color4f fromQt(const QColor&);
    }
}
