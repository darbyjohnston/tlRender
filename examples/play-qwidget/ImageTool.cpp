// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "ImageTool.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>

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
        layout->addStretch();
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
        layout->addStretch();
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
        layout->addStretch();
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
        layout->setMargin(0);
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
        _componentsCheckBox = new QCheckBox(tr("Components"));

        auto layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(0);
        for (size_t i = 0; i < 3; ++i)
        {
            layout->addWidget(_sliders[i]);
        }
        auto hLayout = new QHBoxLayout;
        hLayout->addStretch();
        hLayout->addWidget(_componentsCheckBox);
        layout->addLayout(hLayout);
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

        connect(
            _componentsCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_componentsCallback(bool)));
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

    void ColorSlidersWidget::_componentsCallback(bool value)
    {
        _components = value;
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
        {
            QSignalBlocker signalBlocker(_componentsCheckBox);
            _componentsCheckBox->setChecked(_components);
        }
    }

    ColorWidget::ColorWidget(QWidget* parent) :
        QWidget(parent)
    {
        _colorEnabledCheckBox = new QCheckBox;

        _addSliders = new ColorSlidersWidget;

        _brightnessSliders = new ColorSlidersWidget;
        _brightnessSliders->setRange(math::FloatRange(0.F, 2.F));

        _contrastSliders = new ColorSlidersWidget;
        _contrastSliders->setRange(math::FloatRange(0.F, 2.F));

        _saturationSliders = new ColorSlidersWidget;
        _saturationSliders->setRange(math::FloatRange(0.F, 2.F));

        _tintSlider = new ColorSliderWidget;

        _invertCheckBox = new QCheckBox;

        auto layout = new QFormLayout;
        layout->addRow(tr("Enabled:"), _colorEnabledCheckBox);
        layout->addRow(tr("Add:"), _addSliders);
        layout->addRow(tr("Brightness:"), _brightnessSliders);
        layout->addRow(tr("Contrast:"), _contrastSliders);
        layout->addRow(tr("Saturation:"), _saturationSliders);
        layout->addRow(tr("Tint:"), _tintSlider);
        layout->addRow(tr("Invert:"), _invertCheckBox);
        setLayout(layout);

        _widgetUpdate();

        connect(
            _colorEnabledCheckBox,
            SIGNAL(toggled(bool)),
            SLOT(_colorEnabledCallback(bool)));

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

    void ColorWidget::setValue(const render::Color& value)
    {
        if (value == _value)
            return;
        _value = value;
        _widgetUpdate();
    }

    void ColorWidget::_colorEnabledCallback(bool value)
    {
        _colorEnabled = value;
        Q_EMIT colorEnabledChanged(_colorEnabled);
    }

    void ColorWidget::_addCallback(const math::Vector3f& value)
    {
        _value.add = value;
        Q_EMIT valueChanged(_value);
    }

    void ColorWidget::_brightnessCallback(const math::Vector3f& value)
    {
        _value.brightness = value;
        Q_EMIT valueChanged(_value);
    }

    void ColorWidget::_contrastCallback(const math::Vector3f& value)
    {
        _value.contrast = value;
        Q_EMIT valueChanged(_value);
    }

    void ColorWidget::_saturationCallback(const math::Vector3f& value)
    {
        _value.saturation = value;
        Q_EMIT valueChanged(_value);
    }

    void ColorWidget::_tintCallback(float value)
    {
        _value.tint = value;
        Q_EMIT valueChanged(_value);
    }

    void ColorWidget::_invertCallback(bool value)
    {
        _value.invert = value;
        Q_EMIT valueChanged(_value);
    }

    void ColorWidget::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_colorEnabledCheckBox);
            _colorEnabledCheckBox->setChecked(_colorEnabled);
        }
        {
            QSignalBlocker signalBlocker(_addSliders);
            _addSliders->setValue(_value.add);
        }
        {
            QSignalBlocker signalBlocker(_brightnessSliders);
            _brightnessSliders->setValue(_value.brightness);
        }
        {
            QSignalBlocker signalBlocker(_contrastSliders);
            _contrastSliders->setValue(_value.contrast);
        }
        {
            QSignalBlocker signalBlocker(_saturationSliders);
            _saturationSliders->setValue(_value.saturation);
        }
        {
            QSignalBlocker signalBlocker(_tintSlider);
            _tintSlider->setValue(_value.tint);
        }
        {
            QSignalBlocker signalBlocker(_invertCheckBox);
            _invertCheckBox->setChecked(_value.invert);
        }
    }

    ImageTool::ImageTool(QWidget* parent) :
        QToolBox(parent)
    {
        _yuvRangeWidget = new YUVRangeWidget;
        addItem(_yuvRangeWidget, tr("YUV Range"));

        _channelsWidget = new ChannelsWidget;
        addItem(_channelsWidget, tr("Channels"));

        _alphaBlendWidget = new AlphaBlendWidget;
        addItem(_alphaBlendWidget, tr("Alpha Blend"));

        _colorWidget = new ColorWidget;
        addItem(_colorWidget, tr("Color"));

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
            SIGNAL(valueChanged(const tlr::render::Color&)),
            SLOT(_colorCallback(const tlr::render::Color&)));

        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("ImageTool/CurrentItem").toInt());
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

    void ImageTool::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("ImageTool/CurrentItem", value);
    }

    void ImageTool::_optionsUpdate()
    {
        _yuvRangeWidget->setValue(_imageOptions.yuvRange);
        _channelsWidget->setValue(_imageOptions.channels);
        _alphaBlendWidget->setValue(_imageOptions.alphaBlend);
        _colorWidget->setColorEnabled(_imageOptions.colorEnabled);
        _colorWidget->setValue(_imageOptions.color);
    }
}
