// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "ImageTool.h"

#include <QBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

namespace tlr
{
    YUVRangeWidget::YUVRangeWidget(QWidget* parent) :
        QWidget(parent)
    {
        _comboBox = new QComboBox;
        for (const auto& i : render::getYUVRangeLabels())
        {
            _comboBox->addItem(QString::fromUtf8(i.c_str()));
        }

        auto layout = new QVBoxLayout;
        layout->addWidget(_comboBox);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _comboBox,
            SIGNAL(activated(int)),
            SLOT(_callback(int)));
    }

    void YUVRangeWidget::setValue(render::YUVRange value)
    {
        if (value == _value)
            return;
        _value = value;
        _widgetUpdate();
    }

    void YUVRangeWidget::_callback(int value)
    {
        _value = static_cast<render::YUVRange>(value);
        Q_EMIT valueChanged(_value);
    }

    void YUVRangeWidget::_widgetUpdate()
    {
        QSignalBlocker signalBlocker(_comboBox);
        _comboBox->setCurrentIndex(static_cast<int>(_value));
    }

    ChannelsWidget::ChannelsWidget(QWidget* parent) :
        QWidget(parent)
    {
        _comboBox = new QComboBox;
        for (const auto& i : render::getChannelsLabels())
        {
            _comboBox->addItem(QString::fromUtf8(i.c_str()));
        }

        auto layout = new QVBoxLayout;
        layout->addWidget(_comboBox);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _comboBox,
            SIGNAL(activated(int)),
            SLOT(_callback(int)));
    }

    void ChannelsWidget::setValue(render::Channels value)
    {
        if (value == _value)
            return;
        _value = value;
        _widgetUpdate();
    }

    void ChannelsWidget::_callback(int value)
    {
        _value = static_cast<render::Channels>(value);
        Q_EMIT valueChanged(_value);
    }

    void ChannelsWidget::_widgetUpdate()
    {
        QSignalBlocker signalBlocker(_comboBox);
        _comboBox->setCurrentIndex(static_cast<int>(_value));
    }

    AlphaBlendWidget::AlphaBlendWidget(QWidget* parent) :
        QWidget(parent)
    {
        _comboBox = new QComboBox;
        for (const auto& i : render::getAlphaBlendLabels())
        {
            _comboBox->addItem(QString::fromUtf8(i.c_str()));
        }

        auto layout = new QVBoxLayout;
        layout->addWidget(_comboBox);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _comboBox,
            SIGNAL(activated(int)),
            SLOT(_callback(int)));
    }

    void AlphaBlendWidget::setValue(render::AlphaBlend value)
    {
        if (value == _value)
            return;
        _value = value;
        _widgetUpdate();
    }

    void AlphaBlendWidget::_callback(int value)
    {
        _value = static_cast<render::AlphaBlend>(value);
        Q_EMIT valueChanged(_value);
    }

    void AlphaBlendWidget::_widgetUpdate()
    {
        QSignalBlocker signalBlocker(_comboBox);
        _comboBox->setCurrentIndex(static_cast<int>(_value));
    }

    ColorSliderWidget::ColorSliderWidget(QWidget* parent) :
        QWidget(parent)
    {
        _spinBox = new QDoubleSpinBox;
        _spinBox->setSingleStep(0.1);

        _slider = new QSlider(Qt::Horizontal);
        _slider->setRange(0, 1000);

        auto layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(_spinBox);
        layout->addWidget(_slider, 1);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _spinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_spinBoxCallback(double)));

        connect(
            _slider,
            SIGNAL(valueChanged(int)),
            SLOT(_sliderCallback(int)));
    }

    void ColorSliderWidget::setRange(const math::FloatRange& value)
    {
        if (value == _range)
            return;
        _range = value;
        _widgetUpdate();
    }

    void ColorSliderWidget::setValue(float value)
    {
        if (value == _value)
            return;
        _value = value;
        _widgetUpdate();
    }

    void ColorSliderWidget::_spinBoxCallback(double value)
    {
        _value = value;
        _widgetUpdate();
        Q_EMIT valueChanged(_value);
    }

    void ColorSliderWidget::_sliderCallback(int value)
    {
        _value = value / 1000.F * (_range.getMax() - _range.getMin()) + _range.getMin();
        _widgetUpdate();
        Q_EMIT valueChanged(_value);
    }

    void ColorSliderWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_spinBox);
            _spinBox->setRange(_range.getMin(), _range.getMax());
            _spinBox->setValue(_value);
        }
        {
            QSignalBlocker signalBlocker(_slider);
            _slider->setValue((_value - _range.getMin()) / (_range.getMax() - _range.getMin()) * 1000);
        }
    }

    ColorSlidersWidget::ColorSlidersWidget(QWidget* parent) :
        QWidget(parent)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            _sliders[i] = new ColorSliderWidget;
        }

        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        for (size_t i = 0; i < 3; ++i)
        {
            layout->addWidget(_sliders[i]);
        }
        setLayout(layout);

        _widgetUpdate();

        connect(
            _sliders[0],
            SIGNAL(valueChanged(float)),
            SLOT(_sliderCallback0(float)));
        connect(
            _sliders[1],
            SIGNAL(valueChanged(float)),
            SLOT(_sliderCallback1(float)));
        connect(
            _sliders[2],
            SIGNAL(valueChanged(float)),
            SLOT(_sliderCallback2(float)));
    }

    void ColorSlidersWidget::setRange(const math::FloatRange& value)
    {
        if (value == _range)
            return;
        _range = value;
        _widgetUpdate();
    }

    void ColorSlidersWidget::setValue(const math::Vector3f& value)
    {
        if (value == _value)
            return;
        _value = value;
        _widgetUpdate();
    }

    void ColorSlidersWidget::setComponents(bool value)
    {
        if (value == _components)
            return;
        _components = value;
        _widgetUpdate();
    }

    void ColorSlidersWidget::_sliderCallback0(float value)
    {
        _value.x = value;
        if (!_components)
        {
            _value.y = _value.z = _value.x;
        }
        _widgetUpdate();
        Q_EMIT valueChanged(_value);
    }

    void ColorSlidersWidget::_sliderCallback1(float value)
    {
        _value.y = value;
        if (!_components)
        {
            _value.x = _value.z = _value.y;
        }
        _widgetUpdate();
        Q_EMIT valueChanged(_value);
    }

    void ColorSlidersWidget::_sliderCallback2(float value)
    {
        _value.z = value;
        if (!_components)
        {
            _value.x = _value.y = _value.z;
        }
        _widgetUpdate();
        Q_EMIT valueChanged(_value);
    }

    void ColorSlidersWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_sliders[0]);
            _sliders[0]->setRange(_range);
            _sliders[0]->setValue(_value.x);
        }
        {
            QSignalBlocker signalBlocker(_sliders[1]);
            _sliders[1]->setRange(_range);
            _sliders[1]->setValue(_value.y);
            _sliders[1]->setVisible(_components);
        }
        {
            QSignalBlocker signalBlocker(_sliders[2]);
            _sliders[2]->setRange(_range);
            _sliders[2]->setValue(_value.z);
            _sliders[2]->setVisible(_components);
        }
    }

    ColorWidget::ColorWidget(QWidget* parent) :
        QWidget(parent)
    {
        _colorEnabledCheckBox = new QCheckBox(tr("Enabled"));

        _componentsCheckBox = new QCheckBox(tr("Components"));

        _addSliders = new ColorSlidersWidget;

        _brightnessSliders = new ColorSlidersWidget;
        _brightnessSliders->setRange(math::FloatRange(0.F, 4.F));

        _contrastSliders = new ColorSlidersWidget;
        _contrastSliders->setRange(math::FloatRange(0.F, 4.F));

        _saturationSliders = new ColorSlidersWidget;
        _saturationSliders->setRange(math::FloatRange(0.F, 4.F));

        _tintSlider = new ColorSliderWidget;

        _invertCheckBox = new QCheckBox(tr("Invert"));

        auto layout = new QVBoxLayout;
        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_colorEnabledCheckBox);
        hLayout->addStretch();
        hLayout->addWidget(_componentsCheckBox);
        layout->addLayout(hLayout);
        layout->addWidget(new QLabel(tr("Add")));
        layout->addWidget(_addSliders);
        layout->addWidget(new QLabel(tr("Brightness")));
        layout->addWidget(_brightnessSliders);
        layout->addWidget(new QLabel(tr("Contrast")));
        layout->addWidget(_contrastSliders);
        layout->addWidget(new QLabel(tr("Saturation")));
        layout->addWidget(_saturationSliders);
        layout->addWidget(new QLabel(tr("Tint")));
        layout->addWidget(_tintSlider);
        layout->addWidget(_invertCheckBox);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _colorEnabledCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_colorEnabledCallback(bool)));

        connect(
            _componentsCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_componentsCallback(bool)));

        connect(
            _addSliders,
            SIGNAL(valueChanged(const tlr::math::Vector3f&)),
            SLOT(_addCallback(const tlr::math::Vector3f&)));

        connect(
            _brightnessSliders,
            SIGNAL(valueChanged(const tlr::math::Vector3f&)),
            SLOT(_brightnessCallback(const tlr::math::Vector3f&)));

        connect(
            _contrastSliders,
            SIGNAL(valueChanged(const tlr::math::Vector3f&)),
            SLOT(_contrastCallback(const tlr::math::Vector3f&)));

        connect(
            _saturationSliders,
            SIGNAL(valueChanged(const tlr::math::Vector3f&)),
            SLOT(_saturationCallback(const tlr::math::Vector3f&)));

        connect(
            _tintSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_tintCallback(float)));

        connect(
            _invertCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_invertCallback(bool)));
    }

    void ColorWidget::setColorEnabled(bool value)
    {
        if (value == _colorEnabled)
            return;
        _colorEnabled = value;
        _widgetUpdate();
    }

    void ColorWidget::setColor(const render::Color& value)
    {
        if (value == _color)
            return;
        _color = value;
        _widgetUpdate();
    }

    void ColorWidget::_colorEnabledCallback(bool value)
    {
        _colorEnabled = value;
        Q_EMIT colorEnabledChanged(_colorEnabled);
    }

    void ColorWidget::_componentsCallback(bool value)
    {
        _components = value;
        _widgetUpdate();
    }

    void ColorWidget::_addCallback(const math::Vector3f& value)
    {
        _color.add = value;
        Q_EMIT colorChanged(_color);
    }

    void ColorWidget::_brightnessCallback(const math::Vector3f& value)
    {
        _color.brightness = value;
        Q_EMIT colorChanged(_color);
    }

    void ColorWidget::_contrastCallback(const math::Vector3f& value)
    {
        _color.contrast = value;
        Q_EMIT colorChanged(_color);
    }

    void ColorWidget::_saturationCallback(const math::Vector3f& value)
    {
        _color.saturation = value;
        Q_EMIT colorChanged(_color);
    }

    void ColorWidget::_tintCallback(float value)
    {
        _color.tint = value;
        Q_EMIT colorChanged(_color);
    }

    void ColorWidget::_invertCallback(bool value)
    {
        _color.invert = value;
        Q_EMIT colorChanged(_color);
    }

    void ColorWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_colorEnabledCheckBox);
            _colorEnabledCheckBox->setChecked(_colorEnabled);
        }
        {
            QSignalBlocker signalBlocker(_componentsCheckBox);
            _componentsCheckBox->setChecked(_components);
        }
        {
            QSignalBlocker signalBlocker(_addSliders);
            _addSliders->setComponents(_components);
            _addSliders->setValue(_color.add);
        }
        {
            QSignalBlocker signalBlocker(_brightnessSliders);
            _brightnessSliders->setComponents(_components);
            _brightnessSliders->setValue(_color.brightness);
        }
        {
            QSignalBlocker signalBlocker(_contrastSliders);
            _contrastSliders->setComponents(_components);
            _contrastSliders->setValue(_color.contrast);
        }
        {
            QSignalBlocker signalBlocker(_saturationSliders);
            _saturationSliders->setComponents(_components);
            _saturationSliders->setValue(_color.saturation);
        }
        {
            QSignalBlocker signalBlocker(_tintSlider);
            _tintSlider->setValue(_color.tint);
        }
        {
            QSignalBlocker signalBlocker(_invertCheckBox);
            _invertCheckBox->setChecked(_color.invert);
        }
    }

    LevelsWidget::LevelsWidget(QWidget* parent) :
        QWidget(parent)
    {
        _levelsEnabledCheckBox = new QCheckBox(tr("Enabled"));

        _inLowSlider = new ColorSliderWidget;
        _inHighSlider = new ColorSliderWidget;

        _gammaSlider = new ColorSliderWidget;
        _gammaSlider->setRange(math::FloatRange(.1F, 4.F));

        _outLowSlider = new ColorSliderWidget;
        _outHighSlider = new ColorSliderWidget;

        auto layout = new QVBoxLayout;
        layout->addWidget(_levelsEnabledCheckBox);
        layout->addWidget(new QLabel(tr("In")));
        layout->addWidget(_inLowSlider);
        layout->addWidget(_inHighSlider);
        layout->addWidget(new QLabel(tr("Gamma")));
        layout->addWidget(_gammaSlider);
        layout->addWidget(new QLabel(tr("Out")));
        layout->addWidget(_outLowSlider);
        layout->addWidget(_outHighSlider);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _levelsEnabledCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_levelsEnabledCallback(bool)));

        connect(
            _inLowSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_inLowCallback(float)));
        connect(
            _inHighSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_inHighCallback(float)));

        connect(
            _gammaSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_gammaCallback(float)));

        connect(
            _outLowSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_outLowCallback(float)));
        connect(
            _outHighSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_outHighCallback(float)));
    }

    void LevelsWidget::setLevelsEnabled(bool value)
    {
        if (value == _levelsEnabled)
            return;
        _levelsEnabled = value;
        _widgetUpdate();
    }

    void LevelsWidget::setLevels(const render::Levels& value)
    {
        if (value == _levels)
            return;
        _levels = value;
        _widgetUpdate();
    }

    void LevelsWidget::_levelsEnabledCallback(bool value)
    {
        _levelsEnabled = value;
        Q_EMIT levelsEnabledChanged(_levelsEnabled);
    }

    void LevelsWidget::_inLowCallback(float value)
    {
        _levels.inLow = value;
        Q_EMIT levelsChanged(_levels);
    }

    void LevelsWidget::_inHighCallback(float value)
    {
        _levels.inHigh = value;
        Q_EMIT levelsChanged(_levels);
    }

    void LevelsWidget::_gammaCallback(float value)
    {
        _levels.gamma = value;
        Q_EMIT levelsChanged(_levels);
    }

    void LevelsWidget::_outLowCallback(float value)
    {
        _levels.outLow = value;
        Q_EMIT levelsChanged(_levels);
    }

    void LevelsWidget::_outHighCallback(float value)
    {
        _levels.outHigh = value;
        Q_EMIT levelsChanged(_levels);
    }

    void LevelsWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_levelsEnabledCheckBox);
            _levelsEnabledCheckBox->setChecked(_levelsEnabled);
        }
        {
            QSignalBlocker signalBlocker(_inLowSlider);
            _inLowSlider->setValue(_levels.inLow);
        }
        {
            QSignalBlocker signalBlocker(_inHighSlider);
            _inHighSlider->setValue(_levels.inHigh);
        }
        {
            QSignalBlocker signalBlocker(_gammaSlider);
            _gammaSlider->setValue(_levels.gamma);
        }
        {
            QSignalBlocker signalBlocker(_outLowSlider);
            _outLowSlider->setValue(_levels.outLow);
        }
        {
            QSignalBlocker signalBlocker(_outHighSlider);
            _outHighSlider->setValue(_levels.outHigh);
        }
    }

    ExposureWidget::ExposureWidget(QWidget* parent) :
        QWidget(parent)
    {
        _exposureEnabledCheckBox = new QCheckBox(tr("Enabled"));

        _exposureSlider = new ColorSliderWidget;
        _exposureSlider->setRange(math::FloatRange(-10.F, 10.F));

        _defogSlider = new ColorSliderWidget;
        _defogSlider->setRange(math::FloatRange(0.F, .1F));

        _kneeLowSlider = new ColorSliderWidget;
        _kneeLowSlider->setRange(math::FloatRange(-3.F, 3.F));
        _kneeHighSlider = new ColorSliderWidget;
        _kneeHighSlider->setRange(math::FloatRange(3.5F, 7.5F));

        auto layout = new QVBoxLayout;
        layout->addWidget(_exposureEnabledCheckBox);
        layout->addWidget(_exposureSlider);
        layout->addWidget(new QLabel(tr("Defog")));
        layout->addWidget(_defogSlider);
        layout->addWidget(new QLabel(tr("Knee")));
        layout->addWidget(_kneeLowSlider);
        layout->addWidget(_kneeHighSlider);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _exposureEnabledCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_exposureEnabledCallback(bool)));

        connect(
            _exposureSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_exposureCallback(float)));

        connect(
            _defogSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_defogCallback(float)));

        connect(
            _kneeLowSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_kneeLowCallback(float)));
        connect(
            _kneeHighSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_kneeHighCallback(float)));
    }

    void ExposureWidget::setExposureEnabled(bool value)
    {
        if (value == _exposureEnabled)
            return;
        _exposureEnabled = value;
        _widgetUpdate();
    }

    void ExposureWidget::setExposure(const render::Exposure& value)
    {
        if (value == _exposure)
            return;
        _exposure = value;
        _widgetUpdate();
    }

    void ExposureWidget::_exposureEnabledCallback(bool value)
    {
        _exposureEnabled = value;
        Q_EMIT exposureEnabledChanged(_exposureEnabled);
    }

    void ExposureWidget::_exposureCallback(float value)
    {
        _exposure.exposure = value;
        Q_EMIT exposureChanged(_exposure);
    }

    void ExposureWidget::_defogCallback(float value)
    {
        _exposure.defog = value;
        Q_EMIT exposureChanged(_exposure);
    }

    void ExposureWidget::_kneeLowCallback(float value)
    {
        _exposure.kneeLow = value;
        Q_EMIT exposureChanged(_exposure);
    }

    void ExposureWidget::_kneeHighCallback(float value)
    {
        _exposure.kneeHigh = value;
        Q_EMIT exposureChanged(_exposure);
    }

    void ExposureWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_exposureEnabledCheckBox);
            _exposureEnabledCheckBox->setChecked(_exposureEnabled);
        }
        {
            QSignalBlocker signalBlocker(_exposureSlider);
            _exposureSlider->setValue(_exposure.exposure);
        }
        {
            QSignalBlocker signalBlocker(_defogSlider);
            _defogSlider->setValue(_exposure.defog);
        }
        {
            QSignalBlocker signalBlocker(_kneeLowSlider);
            _kneeLowSlider->setValue(_exposure.kneeLow);
        }
        {
            QSignalBlocker signalBlocker(_kneeHighSlider);
            _kneeHighSlider->setValue(_exposure.kneeHigh);
        }
    }

    SoftClipWidget::SoftClipWidget(QWidget* parent) :
        QWidget(parent)
    {
        _softClipEnabledCheckBox = new QCheckBox(tr("Enabled"));

        _softClipSlider = new ColorSliderWidget;

        auto layout = new QVBoxLayout;
        layout->addWidget(_softClipEnabledCheckBox);
        layout->addWidget(_softClipSlider);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _softClipEnabledCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_softClipEnabledCallback(bool)));

        connect(
            _softClipSlider,
            SIGNAL(valueChanged(float)),
            SLOT(_softClipCallback(float)));
    }

    void SoftClipWidget::setSoftClipEnabled(bool value)
    {
        if (value == _softClipEnabled)
            return;
        _softClipEnabled = value;
        _widgetUpdate();
    }

    void SoftClipWidget::setSoftClip(float value)
    {
        if (value == _softClip)
            return;
        _softClip = value;
        _widgetUpdate();
    }

    void SoftClipWidget::_softClipEnabledCallback(bool value)
    {
        _softClipEnabled = value;
        Q_EMIT softClipEnabledChanged(_softClipEnabled);
    }

    void SoftClipWidget::_softClipCallback(float value)
    {
        _softClip = value;
        Q_EMIT softClipChanged(_softClip);
    }

    void SoftClipWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_softClipEnabledCheckBox);
            _softClipEnabledCheckBox->setChecked(_softClipEnabled);
        }
        {
            QSignalBlocker signalBlocker(_softClipSlider);
            _softClipSlider->setValue(_softClip);
        }
    }

    ImageTool::ImageTool(QWidget* parent) :
        ToolWidget(parent)
    {
        _yuvRangeWidget = new YUVRangeWidget;
        _channelsWidget = new ChannelsWidget;
        _alphaBlendWidget = new AlphaBlendWidget;
        _colorWidget = new ColorWidget;
        _levelsWidget = new LevelsWidget;
        _exposureWidget = new ExposureWidget;
        _softClipWidget = new SoftClipWidget;

        addBellows(tr("YUV Range"), _yuvRangeWidget);
        addBellows(tr("Channels"), _channelsWidget);
        addBellows(tr("Alpha Blend"), _alphaBlendWidget);
        addBellows(tr("Color"), _colorWidget);
        addBellows(tr("Levels"), _levelsWidget);
        addBellows(tr("Exposure"), _exposureWidget);
        addBellows(tr("Soft Clip"), _softClipWidget);
        addStretch();

        _optionsUpdate();

        connect(
            _yuvRangeWidget,
            SIGNAL(valueChanged(tlr::render::YUVRange)),
            SLOT(_yuvRangeCallback(tlr::render::YUVRange)));

        connect(
            _channelsWidget,
            SIGNAL(valueChanged(tlr::render::Channels)),
            SLOT(_channelsCallback(tlr::render::Channels)));

        connect(
            _alphaBlendWidget,
            SIGNAL(valueChanged(tlr::render::AlphaBlend)),
            SLOT(_alphaBlendCallback(tlr::render::AlphaBlend)));

        connect(
            _colorWidget,
            SIGNAL(colorEnabledChanged(bool)),
            SLOT(_colorEnabledCallback(bool)));
        connect(
            _colorWidget,
            SIGNAL(colorChanged(const tlr::render::Color&)),
            SLOT(_colorCallback(const tlr::render::Color&)));

        connect(
            _levelsWidget,
            SIGNAL(levelsEnabledChanged(bool)),
            SLOT(_levelsEnabledCallback(bool)));
        connect(
            _levelsWidget,
            SIGNAL(levelsChanged(const tlr::render::Levels&)),
            SLOT(_levelsCallback(const tlr::render::Levels&)));

        connect(
            _exposureWidget,
            SIGNAL(exposureEnabledChanged(bool)),
            SLOT(_exposureEnabledCallback(bool)));
        connect(
            _exposureWidget,
            SIGNAL(exposureChanged(const tlr::render::Exposure&)),
            SLOT(_exposureCallback(const tlr::render::Exposure&)));

        connect(
            _softClipWidget,
            SIGNAL(softClipEnabledChanged(bool)),
            SLOT(_softClipEnabledCallback(bool)));
        connect(
            _softClipWidget,
            SIGNAL(softClipChanged(float)),
            SLOT(_softClipCallback(float)));
    }

    void ImageTool::setImageOptions(const render::ImageOptions& imageOptions)
    {
        if (imageOptions == _imageOptions)
            return;
        _imageOptions = imageOptions;
        _optionsUpdate();
    }

    void ImageTool::_yuvRangeCallback(render::YUVRange value)
    {
        _imageOptions.yuvRange = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_channelsCallback(render::Channels value)
    {
        _imageOptions.channels = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_alphaBlendCallback(render::AlphaBlend value)
    {
        _imageOptions.alphaBlend = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_colorEnabledCallback(bool value)
    {
        _imageOptions.colorEnabled = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_colorCallback(const render::Color& value)
    {
        _imageOptions.color = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_levelsEnabledCallback(bool value)
    {
        _imageOptions.levelsEnabled = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_levelsCallback(const render::Levels& value)
    {
        _imageOptions.levels = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_exposureEnabledCallback(bool value)
    {
        _imageOptions.exposureEnabled = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_exposureCallback(const render::Exposure& value)
    {
        _imageOptions.exposure = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_softClipEnabledCallback(bool value)
    {
        _imageOptions.softClipEnabled = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_softClipCallback(float value)
    {
        _imageOptions.softClip = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_optionsUpdate()
    {
        _yuvRangeWidget->setValue(_imageOptions.yuvRange);
        _channelsWidget->setValue(_imageOptions.channels);
        _alphaBlendWidget->setValue(_imageOptions.alphaBlend);
        _colorWidget->setColorEnabled(_imageOptions.colorEnabled);
        _colorWidget->setColor(_imageOptions.color);
        _levelsWidget->setLevelsEnabled(_imageOptions.levelsEnabled);
        _levelsWidget->setLevels(_imageOptions.levels);
        _exposureWidget->setExposureEnabled(_imageOptions.exposureEnabled);
        _exposureWidget->setExposure(_imageOptions.exposure);
        _softClipWidget->setSoftClipEnabled(_imageOptions.softClipEnabled);
        _softClipWidget->setSoftClip(_imageOptions.softClip);
    }
}
