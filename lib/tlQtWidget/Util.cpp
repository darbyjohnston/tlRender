// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlQtWidget/Util.h>

namespace tl
{
    namespace qtwidget
    {
        QSize toQt(const ftk::Size2I& value)
        {
            return QSize(value.w, value.h);
        }

        ftk::Size2I fromQt(const QSize& value)
        {
            return ftk::Size2I(value.width(), value.height());
        }

        QColor toQt(const ftk::Color4F& value)
        {
            return QColor::fromRgbF(value.r, value.g, value.b, value.a);
        }

        ftk::Color4F fromQt(const QColor& value)
        {
            return ftk::Color4F(
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
