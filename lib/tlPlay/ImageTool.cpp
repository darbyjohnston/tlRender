// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/ImageTool.h>

#include <tlQWidget/RadioButtonGroup.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>

namespace tl
{
    namespace play
    {
        struct YUVRangeWidget::Private
        {
            render::YUVRange value = tl::render::YUVRange::First;

            qwidget::RadioButtonGroup* radioButtonGroup = nullptr;
        };

        YUVRangeWidget::YUVRangeWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.radioButtonGroup = new qwidget::RadioButtonGroup;
            for (const auto& i : render::getYUVRangeEnums())
            {
                p.radioButtonGroup->addButton(
                    QString::fromUtf8(render::getLabel(i).c_str()),
                    QVariant::fromValue<render::YUVRange>(i));
            }

            auto layout = new QVBoxLayout;
            layout->addWidget(p.radioButtonGroup);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.radioButtonGroup,
                &qwidget::RadioButtonGroup::checked,
                [this](const QVariant& value)
                {
                    Q_EMIT valueChanged(value.value<render::YUVRange>());
                });
        }

        YUVRangeWidget::~YUVRangeWidget()
        {}

        void YUVRangeWidget::setValue(render::YUVRange value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
        }

        void YUVRangeWidget::_widgetUpdate()
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.radioButtonGroup);
            p.radioButtonGroup->setChecked(QVariant::fromValue<render::YUVRange>(p.value));
        }

        struct ChannelsWidget::Private
        {
            render::Channels value = tl::render::Channels::First;

            qwidget::RadioButtonGroup* radioButtonGroup = nullptr;
        };

        ChannelsWidget::ChannelsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.radioButtonGroup = new qwidget::RadioButtonGroup;
            for (const auto& i : render::getChannelsEnums())
            {
                p.radioButtonGroup->addButton(
                    QString::fromUtf8(render::getLabel(i).c_str()),
                    QVariant::fromValue<render::Channels>(i));
            }

            auto layout = new QVBoxLayout;
            layout->addWidget(p.radioButtonGroup);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.radioButtonGroup,
                &qwidget::RadioButtonGroup::checked,
                [this](const QVariant& value)
                {
                    Q_EMIT valueChanged(value.value<render::Channels>());
                });
        }

        ChannelsWidget::~ChannelsWidget()
        {}

        void ChannelsWidget::setValue(render::Channels value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
        }

        void ChannelsWidget::_widgetUpdate()
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.radioButtonGroup);
            p.radioButtonGroup->setChecked(QVariant::fromValue<render::Channels>(p.value));
        }

        struct AlphaBlendWidget::Private
        {
            render::AlphaBlend value = tl::render::AlphaBlend::First;

            qwidget::RadioButtonGroup* radioButtonGroup = nullptr;
        };

        AlphaBlendWidget::AlphaBlendWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.radioButtonGroup = new qwidget::RadioButtonGroup;
            for (const auto& i : render::getAlphaBlendEnums())
            {
                p.radioButtonGroup->addButton(
                    QString::fromUtf8(render::getLabel(i).c_str()),
                    QVariant::fromValue<render::AlphaBlend>(i));
            }

            auto layout = new QVBoxLayout;
            layout->addWidget(p.radioButtonGroup);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.radioButtonGroup,
                &qwidget::RadioButtonGroup::checked,
                [this](const QVariant& value)
                {
                    Q_EMIT valueChanged(value.value<render::AlphaBlend>());
                });
        }

        AlphaBlendWidget::~AlphaBlendWidget()
        {}

        void AlphaBlendWidget::setValue(render::AlphaBlend value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
        }

        void AlphaBlendWidget::_widgetUpdate()
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.radioButtonGroup);
            p.radioButtonGroup->setChecked(QVariant::fromValue<render::AlphaBlend>(p.value));
        }

        namespace
        {
            const size_t sliderSteps = 1000;
        }

        struct ColorSliderWidget::Private
        {
            math::FloatRange range = math::FloatRange(0.F, 1.F);
            float value = 0.F;

            QDoubleSpinBox* spinBox = nullptr;
            QSlider* slider = nullptr;
        };

        ColorSliderWidget::ColorSliderWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.spinBox = new QDoubleSpinBox;
            p.spinBox->setSingleStep(0.1);

            p.slider = new QSlider(Qt::Horizontal);
            p.slider->setRange(0, sliderSteps);

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.spinBox);
            layout->addWidget(p.slider, 1);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.spinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    Q_EMIT valueChanged(value);
                });

            connect(
                p.slider,
                &QSlider::valueChanged,
                [this](int value)
                {
                    Q_EMIT valueChanged(
                        value / static_cast<float>(sliderSteps) *
                        (_p->range.getMax() - _p->range.getMin()) +
                        _p->range.getMin());
                });
        }

        ColorSliderWidget::~ColorSliderWidget()
        {}

        void ColorSliderWidget::setRange(const math::FloatRange& value)
        {
            TLRENDER_P();
            if (value == p.range)
                return;
            p.range = value;
            _widgetUpdate();
        }

        void ColorSliderWidget::setValue(float value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
        }

        void ColorSliderWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.spinBox);
                p.spinBox->setRange(p.range.getMin(), p.range.getMax());
                p.spinBox->setValue(p.value);
            }
            {
                QSignalBlocker signalBlocker(p.slider);
                p.slider->setValue((p.value - p.range.getMin()) / (p.range.getMax() - p.range.getMin()) * sliderSteps);
            }
        }

        struct ColorSlidersWidget::Private
        {
            math::FloatRange range = math::FloatRange(0.F, 1.F);
            math::Vector3f value;

            bool components = false;
            ColorSliderWidget* sliders[3] = { nullptr, nullptr, nullptr };
        };

        ColorSlidersWidget::ColorSlidersWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            for (size_t i = 0; i < 3; ++i)
            {
                p.sliders[i] = new ColorSliderWidget;
            }

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            for (size_t i = 0; i < 3; ++i)
            {
                layout->addWidget(p.sliders[i]);
            }
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.sliders[0],
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    math::Vector3f v = _p->value;
                    v.x = value;
                    if (!_p->components)
                    {
                        v.y = v.z = v.x;
                    }
                    Q_EMIT valueChanged(v);
                });
            connect(
                p.sliders[1],
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    math::Vector3f v = _p->value;
                    v.y = value;
                    if (!_p->components)
                    {
                        v.x = v.z = v.y;
                    }
                    Q_EMIT valueChanged(v);
                });
            connect(
                p.sliders[2],
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    math::Vector3f v = _p->value;
                    v.z = value;
                    if (!_p->components)
                    {
                        v.x = v.y = v.z;
                    }
                    Q_EMIT valueChanged(v);
                });
        }

        ColorSlidersWidget::~ColorSlidersWidget()
        {}

        void ColorSlidersWidget::setRange(const math::FloatRange& value)
        {
            TLRENDER_P();
            if (value == p.range)
                return;
            p.range = value;
            _widgetUpdate();
        }

        void ColorSlidersWidget::setValue(const math::Vector3f& value)
        {
            TLRENDER_P();
            if (value == p.value)
                return;
            p.value = value;
            _widgetUpdate();
        }

        void ColorSlidersWidget::setComponents(bool value)
        {
            TLRENDER_P();
            if (value == p.components)
                return;
            p.components = value;
            _widgetUpdate();
        }

        void ColorSlidersWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.sliders[0]);
                p.sliders[0]->setRange(p.range);
                p.sliders[0]->setValue(p.value.x);
            }
            {
                QSignalBlocker signalBlocker(p.sliders[1]);
                p.sliders[1]->setRange(p.range);
                p.sliders[1]->setValue(p.value.y);
                p.sliders[1]->setVisible(p.components);
            }
            {
                QSignalBlocker signalBlocker(p.sliders[2]);
                p.sliders[2]->setRange(p.range);
                p.sliders[2]->setValue(p.value.z);
                p.sliders[2]->setVisible(p.components);
            }
        }

        struct ColorWidget::Private
        {
            bool colorEnabled = false;
            render::Color color;
            bool components = false;

            QCheckBox* colorEnabledCheckBox = nullptr;
            QCheckBox* componentsCheckBox = nullptr;
            ColorSlidersWidget* addSliders = nullptr;
            ColorSlidersWidget* brightnessSliders = nullptr;
            ColorSlidersWidget* contrastSliders = nullptr;
            ColorSlidersWidget* saturationSliders = nullptr;
            ColorSliderWidget* tintSlider = nullptr;
            QCheckBox* invertCheckBox = nullptr;
        };

        ColorWidget::ColorWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.colorEnabledCheckBox = new QCheckBox(tr("Enabled"));

            p.componentsCheckBox = new QCheckBox(tr("Components"));

            p.addSliders = new ColorSlidersWidget;
            p.addSliders->setRange(math::FloatRange(-1.F, 1.F));

            p.brightnessSliders = new ColorSlidersWidget;
            p.brightnessSliders->setRange(math::FloatRange(0.F, 4.F));

            p.contrastSliders = new ColorSlidersWidget;
            p.contrastSliders->setRange(math::FloatRange(0.F, 4.F));

            p.saturationSliders = new ColorSlidersWidget;
            p.saturationSliders->setRange(math::FloatRange(0.F, 4.F));

            p.tintSlider = new ColorSliderWidget;

            p.invertCheckBox = new QCheckBox(tr("Invert"));

            auto layout = new QVBoxLayout;
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.colorEnabledCheckBox);
            hLayout->addStretch();
            hLayout->addWidget(p.componentsCheckBox);
            layout->addLayout(hLayout);
            layout->addWidget(new QLabel(tr("Add")));
            layout->addWidget(p.addSliders);
            layout->addWidget(new QLabel(tr("Brightness")));
            layout->addWidget(p.brightnessSliders);
            layout->addWidget(new QLabel(tr("Contrast")));
            layout->addWidget(p.contrastSliders);
            layout->addWidget(new QLabel(tr("Saturation")));
            layout->addWidget(p.saturationSliders);
            layout->addWidget(new QLabel(tr("Tint")));
            layout->addWidget(p.tintSlider);
            layout->addWidget(p.invertCheckBox);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.colorEnabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT colorEnabledChanged(value);
                });

            connect(
                p.componentsCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    _p->components = value;
                    _widgetUpdate();
                });

            connect(
                p.addSliders,
                &ColorSlidersWidget::valueChanged,
                [this](const tl::math::Vector3f& value)
                {
                    tl::render::Color color = _p->color;
                    color.add = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.brightnessSliders,
                &ColorSlidersWidget::valueChanged,
                [this](const tl::math::Vector3f& value)
                {
                    tl::render::Color color = _p->color;
                    color.brightness = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.contrastSliders,
                &ColorSlidersWidget::valueChanged,
                [this](const tl::math::Vector3f& value)
                {
                    tl::render::Color color = _p->color;
                    color.contrast = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.saturationSliders,
                &ColorSlidersWidget::valueChanged,
                [this](const tl::math::Vector3f& value)
                {
                    tl::render::Color color = _p->color;
                    color.saturation = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.tintSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    tl::render::Color color = _p->color;
                    color.tint = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.invertCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    tl::render::Color color = _p->color;
                    color.invert = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });
        }

        ColorWidget::~ColorWidget()
        {}

        void ColorWidget::setColorEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.colorEnabled)
                return;
            p.colorEnabled = value;
            _widgetUpdate();
        }

        void ColorWidget::setColor(const render::Color& value)
        {
            TLRENDER_P();
            if (value == p.color)
                return;
            p.color = value;
            _widgetUpdate();
        }

        void ColorWidget::_componentsCallback(bool value)
        {
            TLRENDER_P();
            p.components = value;
            _widgetUpdate();
        }

        void ColorWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.colorEnabledCheckBox);
                p.colorEnabledCheckBox->setChecked(p.colorEnabled);
            }
            {
                QSignalBlocker signalBlocker(p.componentsCheckBox);
                p.componentsCheckBox->setChecked(p.components);
            }
            {
                QSignalBlocker signalBlocker(p.addSliders);
                p.addSliders->setComponents(p.components);
                p.addSliders->setValue(p.color.add);
            }
            {
                QSignalBlocker signalBlocker(p.brightnessSliders);
                p.brightnessSliders->setComponents(p.components);
                p.brightnessSliders->setValue(p.color.brightness);
            }
            {
                QSignalBlocker signalBlocker(p.contrastSliders);
                p.contrastSliders->setComponents(p.components);
                p.contrastSliders->setValue(p.color.contrast);
            }
            {
                QSignalBlocker signalBlocker(p.saturationSliders);
                p.saturationSliders->setComponents(p.components);
                p.saturationSliders->setValue(p.color.saturation);
            }
            {
                QSignalBlocker signalBlocker(p.tintSlider);
                p.tintSlider->setValue(p.color.tint);
            }
            {
                QSignalBlocker signalBlocker(p.invertCheckBox);
                p.invertCheckBox->setChecked(p.color.invert);
            }
        }

        struct LevelsWidget::Private
        {
            bool levelsEnabled = false;
            render::Levels levels;

            QCheckBox* levelsEnabledCheckBox = nullptr;
            ColorSliderWidget* inLowSlider = nullptr;
            ColorSliderWidget* inHighSlider = nullptr;
            ColorSliderWidget* gammaSlider = nullptr;
            ColorSliderWidget* outLowSlider = nullptr;
            ColorSliderWidget* outHighSlider = nullptr;
        };

        LevelsWidget::LevelsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.levelsEnabledCheckBox = new QCheckBox(tr("Enabled"));

            p.inLowSlider = new ColorSliderWidget;
            p.inHighSlider = new ColorSliderWidget;

            p.gammaSlider = new ColorSliderWidget;
            p.gammaSlider->setRange(math::FloatRange(.1F, 4.F));

            p.outLowSlider = new ColorSliderWidget;
            p.outHighSlider = new ColorSliderWidget;

            auto layout = new QVBoxLayout;
            layout->addWidget(p.levelsEnabledCheckBox);
            layout->addWidget(new QLabel(tr("In")));
            layout->addWidget(p.inLowSlider);
            layout->addWidget(p.inHighSlider);
            layout->addWidget(new QLabel(tr("Gamma")));
            layout->addWidget(p.gammaSlider);
            layout->addWidget(new QLabel(tr("Out")));
            layout->addWidget(p.outLowSlider);
            layout->addWidget(p.outHighSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.levelsEnabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT levelsEnabledChanged(value);
                });

            connect(
                p.inLowSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Levels levels = _p->levels;
                    levels.inLow = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });
            connect(
                p.inHighSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Levels levels = _p->levels;
                    levels.inHigh = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });

            connect(
                p.gammaSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Levels levels = _p->levels;
                    levels.gamma = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });

            connect(
                p.outLowSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Levels levels = _p->levels;
                    levels.outLow = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });
            connect(
                p.outHighSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Levels levels = _p->levels;
                    levels.outHigh = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });
        }

        LevelsWidget::~LevelsWidget()
        {}

        void LevelsWidget::setLevelsEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.levelsEnabled)
                return;
            p.levelsEnabled = value;
            _widgetUpdate();
        }

        void LevelsWidget::setLevels(const render::Levels& value)
        {
            TLRENDER_P();
            if (value == p.levels)
                return;
            p.levels = value;
            _widgetUpdate();
        }

        void LevelsWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.levelsEnabledCheckBox);
                p.levelsEnabledCheckBox->setChecked(p.levelsEnabled);
            }
            {
                QSignalBlocker signalBlocker(p.inLowSlider);
                p.inLowSlider->setValue(p.levels.inLow);
            }
            {
                QSignalBlocker signalBlocker(p.inHighSlider);
                p.inHighSlider->setValue(p.levels.inHigh);
            }
            {
                QSignalBlocker signalBlocker(p.gammaSlider);
                p.gammaSlider->setValue(p.levels.gamma);
            }
            {
                QSignalBlocker signalBlocker(p.outLowSlider);
                p.outLowSlider->setValue(p.levels.outLow);
            }
            {
                QSignalBlocker signalBlocker(p.outHighSlider);
                p.outHighSlider->setValue(p.levels.outHigh);
            }
        }

        struct ExposureWidget::Private
        {
            bool exposureEnabled = false;
            render::Exposure exposure;

            QCheckBox* exposureEnabledCheckBox = nullptr;
            ColorSliderWidget* exposureSlider = nullptr;
            ColorSliderWidget* defogSlider = nullptr;
            ColorSliderWidget* kneeLowSlider = nullptr;
            ColorSliderWidget* kneeHighSlider = nullptr;
        };

        ExposureWidget::ExposureWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.exposureEnabledCheckBox = new QCheckBox(tr("Enabled"));

            p.exposureSlider = new ColorSliderWidget;
            p.exposureSlider->setRange(math::FloatRange(-10.F, 10.F));

            p.defogSlider = new ColorSliderWidget;
            p.defogSlider->setRange(math::FloatRange(0.F, .1F));

            p.kneeLowSlider = new ColorSliderWidget;
            p.kneeLowSlider->setRange(math::FloatRange(-3.F, 3.F));
            p.kneeHighSlider = new ColorSliderWidget;
            p.kneeHighSlider->setRange(math::FloatRange(3.5F, 7.5F));

            auto layout = new QVBoxLayout;
            layout->addWidget(p.exposureEnabledCheckBox);
            layout->addWidget(p.exposureSlider);
            layout->addWidget(new QLabel(tr("Defog")));
            layout->addWidget(p.defogSlider);
            layout->addWidget(new QLabel(tr("Knee")));
            layout->addWidget(p.kneeLowSlider);
            layout->addWidget(p.kneeHighSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.exposureEnabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT exposureEnabledChanged(value);
                });

            connect(
                p.exposureSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Exposure exposure = _p->exposure;
                    exposure.exposure = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });

            connect(
                p.defogSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Exposure exposure = _p->exposure;
                    exposure.defog = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });

            connect(
                p.kneeLowSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Exposure exposure = _p->exposure;
                    exposure.kneeLow = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });
            connect(
                p.kneeHighSlider, &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    render::Exposure exposure = _p->exposure;
                    exposure.kneeHigh = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });

        }

        ExposureWidget::~ExposureWidget()
        {}

        void ExposureWidget::setExposureEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.exposureEnabled)
                return;
            p.exposureEnabled = value;
            _widgetUpdate();
        }

        void ExposureWidget::setExposure(const render::Exposure& value)
        {
            TLRENDER_P();
            if (value == p.exposure)
                return;
            p.exposure = value;
            _widgetUpdate();
        }

        void ExposureWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.exposureEnabledCheckBox);
                p.exposureEnabledCheckBox->setChecked(p.exposureEnabled);
            }
            {
                QSignalBlocker signalBlocker(p.exposureSlider);
                p.exposureSlider->setValue(p.exposure.exposure);
            }
            {
                QSignalBlocker signalBlocker(p.defogSlider);
                p.defogSlider->setValue(p.exposure.defog);
            }
            {
                QSignalBlocker signalBlocker(p.kneeLowSlider);
                p.kneeLowSlider->setValue(p.exposure.kneeLow);
            }
            {
                QSignalBlocker signalBlocker(p.kneeHighSlider);
                p.kneeHighSlider->setValue(p.exposure.kneeHigh);
            }
        }

        struct SoftClipWidget::Private
        {
            bool softClipEnabled = false;
            float softClip = 0.F;

            QCheckBox* softClipEnabledCheckBox = nullptr;
            ColorSliderWidget* softClipSlider = nullptr;
        };

        SoftClipWidget::SoftClipWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.softClipEnabledCheckBox = new QCheckBox(tr("Enabled"));

            p.softClipSlider = new ColorSliderWidget;

            auto layout = new QVBoxLayout;
            layout->addWidget(p.softClipEnabledCheckBox);
            layout->addWidget(p.softClipSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.softClipEnabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT softClipEnabledChanged(value);
                });

            connect(
                p.softClipSlider,
                &ColorSliderWidget::valueChanged,
                [this](float value)
                {
                    Q_EMIT softClipChanged(value);
                    Q_EMIT softClipEnabledChanged(true);
                });
        }

        SoftClipWidget::~SoftClipWidget()
        {}

        void SoftClipWidget::setSoftClipEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.softClipEnabled)
                return;
            p.softClipEnabled = value;
            _widgetUpdate();
        }

        void SoftClipWidget::setSoftClip(float value)
        {
            TLRENDER_P();
            if (value == p.softClip)
                return;
            p.softClip = value;
            _widgetUpdate();
        }

        void SoftClipWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.softClipEnabledCheckBox);
                p.softClipEnabledCheckBox->setChecked(p.softClipEnabled);
            }
            {
                QSignalBlocker signalBlocker(p.softClipSlider);
                p.softClipSlider->setValue(p.softClip);
            }
        }

        struct ImageTool::Private
        {
            render::ImageOptions imageOptions;

            YUVRangeWidget* yuvRangeWidget = nullptr;
            ChannelsWidget* channelsWidget = nullptr;
            AlphaBlendWidget* alphaBlendWidget = nullptr;
            ColorWidget* colorWidget = nullptr;
            LevelsWidget* levelsWidget = nullptr;
            ExposureWidget* exposureWidget = nullptr;
            SoftClipWidget* softClipWidget = nullptr;
        };

        ImageTool::ImageTool(QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.yuvRangeWidget = new YUVRangeWidget;
            p.channelsWidget = new ChannelsWidget;
            p.alphaBlendWidget = new AlphaBlendWidget;
            p.colorWidget = new ColorWidget;
            p.levelsWidget = new LevelsWidget;
            p.exposureWidget = new ExposureWidget;
            p.softClipWidget = new SoftClipWidget;

            addBellows(tr("YUV Range"), p.yuvRangeWidget);
            addBellows(tr("Channels"), p.channelsWidget);
            addBellows(tr("Alpha Blend"), p.alphaBlendWidget);
            addBellows(tr("Color"), p.colorWidget);
            addBellows(tr("Levels"), p.levelsWidget);
            addBellows(tr("Exposure"), p.exposureWidget);
            addBellows(tr("Soft Clip"), p.softClipWidget);
            addStretch();

            _optionsUpdate();

            connect(
                p.yuvRangeWidget,
                &YUVRangeWidget::valueChanged,
                [this](tl::render::YUVRange value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.yuvRange = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });

            connect(
                p.channelsWidget,
                &ChannelsWidget::valueChanged,
                [this](tl::render::Channels value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.channels = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });

            connect(
                p.alphaBlendWidget,
                &AlphaBlendWidget::valueChanged,
                [this](tl::render::AlphaBlend value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.alphaBlend = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });

            connect(
                p.colorWidget,
                &ColorWidget::colorEnabledChanged,
                [this](bool value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.colorEnabled = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });
            connect(
                p.colorWidget,
                &ColorWidget::colorChanged,
                [this](const tl::render::Color& value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.color = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });

            connect(
                p.levelsWidget,
                &LevelsWidget::levelsEnabledChanged,
                [this](bool value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.levelsEnabled = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });
            connect(
                p.levelsWidget,
                &LevelsWidget::levelsChanged,
                [this](const tl::render::Levels& value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.levels = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });

            connect(
                p.exposureWidget,
                &ExposureWidget::exposureEnabledChanged,
                [this](bool value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.exposureEnabled = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });
            connect(
                p.exposureWidget,
                &ExposureWidget::exposureChanged,
                [this](const tl::render::Exposure& value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.exposure = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });

            connect(
                p.softClipWidget,
                &SoftClipWidget::softClipEnabledChanged,
                [this](bool value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.softClipEnabled = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });
            connect(
                p.softClipWidget,
                &SoftClipWidget::softClipChanged,
                [this](float value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.softClip = value;
                    Q_EMIT imageOptionsChanged(imageOptions);
                });
        }

        ImageTool::~ImageTool()
        {}

        void ImageTool::setImageOptions(const render::ImageOptions& imageOptions)
        {
            TLRENDER_P();
            if (imageOptions == p.imageOptions)
                return;
            p.imageOptions = imageOptions;
            _optionsUpdate();
        }

        void ImageTool::_optionsUpdate()
        {
            TLRENDER_P();
            p.yuvRangeWidget->setValue(p.imageOptions.yuvRange);
            p.channelsWidget->setValue(p.imageOptions.channels);
            p.alphaBlendWidget->setValue(p.imageOptions.alphaBlend);
            p.colorWidget->setColorEnabled(p.imageOptions.colorEnabled);
            p.colorWidget->setColor(p.imageOptions.color);
            p.levelsWidget->setLevelsEnabled(p.imageOptions.levelsEnabled);
            p.levelsWidget->setLevels(p.imageOptions.levels);
            p.exposureWidget->setExposureEnabled(p.imageOptions.exposureEnabled);
            p.exposureWidget->setExposure(p.imageOptions.exposure);
            p.softClipWidget->setSoftClipEnabled(p.imageOptions.softClipEnabled);
            p.softClipWidget->setSoftClip(p.imageOptions.softClip);
        }
    }
}
