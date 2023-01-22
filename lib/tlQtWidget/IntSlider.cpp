// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/IntSlider.h>

#include <tlQtWidget/Util.h>

#include <QBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>

namespace tl
{
    namespace qtwidget
    {
        struct IntSlider::Private
        {
            math::IntRange range = math::IntRange(0, 100);
            int value = 0;
            int defaultValue = -1;
            int singleStep = 1;
            int pageStep = 10;
            Qt::Orientation orientation = Qt::Horizontal;
            QSpinBox* spinBox = nullptr;
            QSlider* slider = nullptr;
            QToolButton* defaultValueButton = nullptr;
            QBoxLayout* layout = nullptr;
        };

        IntSlider::IntSlider(Qt::Orientation orientation, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.spinBox = new QSpinBox;
            p.spinBox->setFont(QFont("Noto Mono"));

            p.defaultValueButton = new QToolButton;
            p.defaultValueButton->setAutoRaise(true);
            p.defaultValueButton->setIcon(QIcon(":/Icons/Reset.svg"));
            p.defaultValueButton->setToolTip(tr("Reset to the default value"));

            _layoutUpdate();
            _widgetUpdate();

            connect(
                p.spinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    _p->value = value;
                    _widgetUpdate();
                    Q_EMIT valueChanged(_p->value);
                });

            connect(
                p.defaultValueButton,
                &QToolButton::clicked,
                [this]
                {
                    setValue(_p->defaultValue);
                });
        }

        IntSlider::~IntSlider()
        {}

        const math::IntRange& IntSlider::range() const
        {
            return _p->range;
        }

        int IntSlider::value() const
        {
            return _p->value;
        }

        int IntSlider::defaultValue() const
        {
            return _p->defaultValue;
        }

        int IntSlider::singleStep() const
        {
            return _p->singleStep;
        }

        int IntSlider::pageStep() const
        {
            return _p->pageStep;
        }

        Qt::Orientation IntSlider::orientation() const
        {
            return _p->orientation;
        }

        void IntSlider::setRange(const math::IntRange& value)
        {
            TLRENDER_P();
            if (value == p.range)
                return;
            p.range = value;
            _widgetUpdate();
            Q_EMIT rangeChanged(p.range);
        }

        void IntSlider::setValue(int value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
            Q_EMIT valueChanged(p.value);
        }

        void IntSlider::setDefaultValue(int value)
        {
            TLRENDER_P();
            if (value == p.defaultValue)
                return;
            p.defaultValue = value;
            _widgetUpdate();
        }

        void IntSlider::setSingleStep(int value)
        {
            TLRENDER_P();
            if (value == p.singleStep)
                return;
            p.singleStep = value;
            _widgetUpdate();
        }

        void IntSlider::setPageStep(int value)
        {
            TLRENDER_P();
            if (value == p.pageStep)
                return;
            p.pageStep = value;
            _widgetUpdate();
        }

        void IntSlider::setOrientation(Qt::Orientation value)
        {
            TLRENDER_P();
            if (value == p.orientation)
                return;
            p.orientation = value;
            _layoutUpdate();
        }

        void IntSlider::_layoutUpdate()
        {
            TLRENDER_P();

            if (p.layout)
            {
                p.layout->removeWidget(p.spinBox);
                p.layout->removeWidget(p.slider);
                p.layout->removeWidget(p.defaultValueButton);
            }
            delete p.slider;
            delete p.layout;

            p.slider = new QSlider(p.orientation);
            switch (p.orientation)
            {
            case Qt::Horizontal:
                p.layout = new QHBoxLayout;
                break;
            case Qt::Vertical:
                p.layout = new QVBoxLayout;
                break;
            default: break;
            }
            p.layout->setContentsMargins(0, 0, 0, 0);

            p.layout->addWidget(p.spinBox);
            p.layout->addWidget(p.slider, 1);
            p.layout->addWidget(p.defaultValueButton);
            setLayout(p.layout);

            connect(
                p.slider,
                &QSlider::valueChanged,
                [this](int value)
                {
                    _p->value = value;
                    _widgetUpdate();
                    Q_EMIT valueChanged(_p->value);
                });
        }

        void IntSlider::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.spinBox);
                p.spinBox->setRange(p.range.getMin(), p.range.getMax());
                p.spinBox->setValue(p.value);
                p.spinBox->setSingleStep(p.singleStep);
            }
            {
                QSignalBlocker blocker(p.slider);
                p.slider->setRange(p.range.getMin(), p.range.getMax());
                p.slider->setValue(p.value);
                p.slider->setSingleStep(p.singleStep);
                p.slider->setPageStep(p.pageStep);
            }
            p.defaultValueButton->setVisible(p.range.contains(p.defaultValue));
            p.defaultValueButton->setEnabled(p.value != p.defaultValue);
        }
    }
}
