// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Util.h>

namespace tl
{
    namespace qtwidget
    {
        QSize toQt(const feather_tk::Size2I& value)
        {
            return QSize(value.w, value.h);
        }

        feather_tk::Size2I fromQt(const QSize& value)
        {
            return feather_tk::Size2I(value.width(), value.height());
        }

        QColor toQt(const feather_tk::Color4F& value)
        {
            return QColor::fromRgbF(value.r, value.g, value.b, value.a);
        }

        feather_tk::Color4F fromQt(const QColor& value)
        {
            return feather_tk::Color4F(
                value.redF(),
                value.greenF(),
                value.blueF(),
                value.alphaF());
        }

        void setFloatOnTop(bool value, QWidget* window)
        {
            if (value && !(window->windowFlags() & Qt::WindowStaysOnTopHint))
            {
                window->setWindowFlags(window->windowFlags() | Qt::WindowStaysOnTopHint);
                window->show();
            }
            else if (!value && (window->windowFlags() & Qt::WindowStaysOnTopHint))
            {
                window->setWindowFlags(window->windowFlags() & ~Qt::WindowStaysOnTopHint);
                window->show();
            }
        }
    }
}
