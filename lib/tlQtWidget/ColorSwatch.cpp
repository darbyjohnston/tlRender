// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ColorSwatch.h>

#include <tlQtWidget/ColorDialog.h>
#include <tlQtWidget/Util.h>

#include <QMouseEvent>
#include <QPainter>

namespace tl
{
    namespace qtwidget
    {
        struct ColorSwatch::Private
        {
            dtk::Color4F color;
            int swatchSize = 20;
            bool editable = false;
        };

        ColorSwatch::ColorSwatch(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        }

        ColorSwatch::~ColorSwatch()
        {}

        const dtk::Color4F& ColorSwatch::color() const
        {
            return _p->color;
        }

        void ColorSwatch::setSwatchSize(int value)
        {
            DTK_P();
            if (value == p.swatchSize)
                return;
            p.swatchSize = value;
            updateGeometry();
        }

        void ColorSwatch::setEditable(bool value)
        {
            _p->editable = value;
        }

        QSize ColorSwatch::minimumSizeHint() const
        {
            DTK_P();
            return QSize(p.swatchSize, p.swatchSize);
        }

        void ColorSwatch::setColor(const dtk::Color4F& value)
        {
            DTK_P();
            if (value == p.color)
                return;
            p.color = value;
            update();
            Q_EMIT colorChanged(p.color);
        }

        void ColorSwatch::paintEvent(QPaintEvent*)
        {
            DTK_P();
            QPainter painter(this);
            painter.fillRect(0, 0, width(), height(), toQt(p.color));
        }

        void ColorSwatch::mousePressEvent(QMouseEvent* event)
        {
            DTK_P();
            if (p.editable)
            {
                event->accept();
                QScopedPointer<ColorDialog> dialog(new ColorDialog(p.color));
                if (QDialog::Accepted == dialog->exec())
                {
                    setColor(dialog->color());
                }
            }
        }

        void ColorSwatch::mouseReleaseEvent(QMouseEvent* event)
        {
            DTK_P();
            if (p.editable)
            {
                event->accept();
            }
        }
    }
}
