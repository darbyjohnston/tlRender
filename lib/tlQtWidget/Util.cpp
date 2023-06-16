// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Util.h>

namespace tl
{
    namespace qtwidget
    {
        imaging::Color4f fromQt(const QColor& value)
        {
            return imaging::Color4f(
                value.redF(),
                value.greenF(),
                value.blueF(),
                value.alphaF());
        }
    }
}
