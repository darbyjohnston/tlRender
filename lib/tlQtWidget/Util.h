// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>

#include <QColor>

namespace tl
{
    namespace qtwidget
    {
        //! Convert a Qt color.
        image::Color4f fromQt(const QColor&);
    }
}
