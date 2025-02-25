// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ColorWidget.h>

#include <tlQtWidget/ColorSwatch.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <QVBoxLayout>

namespace tl
{
    namespace qtwidget
    {
        struct ColorWidget::Private
        {
            dtk::Color4F color;

            ColorSwatch* swatch = nullptr;
            std::vector<FloatEditSlider*> sliders;
        };

        ColorWidget::ColorWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.swatch = new ColorSwatch;
            p.swatch->setSwatchSize(40);
            p.swatch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

            for (size_t i = 0; i < 4; ++i)
            {
                auto slider = new FloatEditSlider;
                p.sliders.push_back(slider);
            }

            auto layout = new QHBoxLayout;
            layout->addWidget(p.swatch);
            auto vLayout = new QVBoxLayout;
            for (size_t i = 0; i < p.sliders.size(); ++i)
            {
                vLayout->addWidget(p.sliders[i]);
            }
            layout->addLayout(vLayout);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.sliders[0],
                &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    setColor(dtk::Color4F(
                        value,
                        _p->color.g,
                        _p->color.b,
                        _p->color.a));
                });
            connect(
                p.sliders[1],
                &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    setColor(dtk::Color4F(
                        _p->color.r,
                        value,
                        _p->color.b,
                        _p->color.a));
                });
            connect(
                p.sliders[2],
                &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    setColor(dtk::Color4F(
                        _p->color.r,
                        _p->color.g,
                        value,
                        _p->color.a));
                });
            connect(
                p.sliders[3],
                &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    setColor(dtk::Color4F(
                        _p->color.r,
                        _p->color.g,
                        _p->color.b,
                        value));
                });
        }

        ColorWidget::~ColorWidget()
        {}

        const dtk::Color4F& ColorWidget::color() const
        {
            return _p->color;
        }

        void ColorWidget::setColor(const dtk::Color4F& value)
        {
            DTK_P();
            if (value == p.color)
                return;
            p.color = value;
            _widgetUpdate();
            Q_EMIT colorChanged(p.color);
        }

        void ColorWidget::_widgetUpdate()
        {
            DTK_P();
            {
                QSignalBlocker signalBlocker(p.swatch);
                p.swatch->setColor(p.color);
            }
            {
                QSignalBlocker signalBlocker(p.sliders[0]);
                p.sliders[0]->setValue(p.color.r);
            }
            {
                QSignalBlocker signalBlocker(p.sliders[1]);
                p.sliders[1]->setValue(p.color.g);
            }
            {
                QSignalBlocker signalBlocker(p.sliders[2]);
                p.sliders[2]->setValue(p.color.b);
            }
            {
                QSignalBlocker signalBlocker(p.sliders[3]);
                p.sliders[3]->setValue(p.color.a);
            }
        }
    }
}
