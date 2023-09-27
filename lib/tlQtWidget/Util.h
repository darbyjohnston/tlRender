// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Size.h>

#include <QColor>
#include <QSize>

namespace tl
{
    namespace qtwidget
    {
        //! Convert to a Qt size.
        QSize toQt(const math::Size2i&);

        //! Convert from a Qt size.
        math::Size2i fromQt(const QSize&);

        //! Convert to a Qt color.
        QColor toQt(const image::Color4f&);

        //! Convert from a Qt color.
        image::Color4f fromQt(const QColor&);
    }
}
