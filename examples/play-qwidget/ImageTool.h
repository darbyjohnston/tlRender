// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/IRender.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QToolBox>

namespace tlr
{
    //! YUV range widget.
    class YUVRangeWidget : public QWidget
    {
        Q_OBJECT

    public:
        YUVRangeWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setValue(tlr::render::YUVRange);

    Q_SIGNALS:
        void valueChanged(tlr::render::YUVRange);

    private Q_SLOTS:
        void _callback(int);

    private:
        void _widgetUpdate();

        render::YUVRange _value = tlr::render::YUVRange::First;

        QComboBox* _comboBox = nullptr;
    };

    //! Channels widget.
    class ChannelsWidget : public QWidget
    {
        Q_OBJECT

    public:
        ChannelsWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setValue(tlr::render::Channels);

    Q_SIGNALS:
        void valueChanged(tlr::render::Channels);

    private Q_SLOTS:
        void _callback(int);

    private:
        void _widgetUpdate();

        render::Channels _value = tlr::render::Channels::First;

        QComboBox* _comboBox = nullptr;
    };

    //! Alpha blend widget.
    class AlphaBlendWidget : public QWidget
    {
        Q_OBJECT

    public:
        AlphaBlendWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setValue(tlr::render::AlphaBlend);

    Q_SIGNALS:
        void valueChanged(tlr::render::AlphaBlend);

    private Q_SLOTS:
        void _callback(int);

    private:
        void _widgetUpdate();

        render::AlphaBlend _value = tlr::render::AlphaBlend::First;

        QComboBox* _comboBox = nullptr;
    };

    //! Color slider widget.
    class ColorSliderWidget : public QWidget
    {
        Q_OBJECT

    public:
        ColorSliderWidget(QWidget* parent = nullptr);

        void setRange(const math::FloatRange&);

    public Q_SLOTS:
        void setValue(float);

    Q_SIGNALS:
        void valueChanged(float);

    private Q_SLOTS:
        void _spinBoxCallback(double);
        void _sliderCallback(int);

    private:
        void _widgetUpdate();

        math::FloatRange _range = math::FloatRange(0.F, 1.F);
        float _value = 0.F;

        QDoubleSpinBox* _spinBox = nullptr;
        QSlider* _slider = nullptr;
    };

    //! Color sliders widget.
    class ColorSlidersWidget : public QWidget
    {
        Q_OBJECT

    public:
        ColorSlidersWidget(QWidget* parent = nullptr);

        void setRange(const math::FloatRange&);

    public Q_SLOTS:
        void setValue(const tlr::math::Vector3f&);

    Q_SIGNALS:
        void valueChanged(const tlr::math::Vector3f&);

    private Q_SLOTS:
        void _sliderCallback0(float);
        void _sliderCallback1(float);
        void _sliderCallback2(float);
        void _componentsCallback(bool);

    private:
        void _widgetUpdate();

        math::FloatRange _range = math::FloatRange(0.F, 1.F);
        math::Vector3f _value;
        bool _components = false;

        ColorSliderWidget* _sliders[3] = { nullptr, nullptr, nullptr };
        QCheckBox* _componentsCheckBox = nullptr;
    };

    //! Color widget.
    class ColorWidget : public QWidget
    {
        Q_OBJECT

    public:
        ColorWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setColorEnabled(bool);
        void setValue(const tlr::render::Color&);

    Q_SIGNALS:
        void colorEnabledChanged(bool);
        void valueChanged(const tlr::render::Color&);

    private Q_SLOTS:
        void _colorEnabledCallback(bool);
        void _addCallback(const tlr::math::Vector3f&);
        void _brightnessCallback(const tlr::math::Vector3f&);
        void _contrastCallback(const tlr::math::Vector3f&);
        void _saturationCallback(const tlr::math::Vector3f&);
        void _tintCallback(float);
        void _invertCallback(bool);

    private:
        void _widgetUpdate();

        bool _colorEnabled = false;
        render::Color _value;

        QCheckBox* _colorEnabledCheckBox = nullptr;
        ColorSlidersWidget* _addSliders = nullptr;
        ColorSlidersWidget* _brightnessSliders = nullptr;
        ColorSlidersWidget* _contrastSliders = nullptr;
        ColorSlidersWidget* _saturationSliders = nullptr;
        ColorSliderWidget* _tintSlider = nullptr;
        QCheckBox* _invertCheckBox = nullptr;
    };

    //! Image tool.
    class ImageTool : public QToolBox
    {
        Q_OBJECT

    public:
        ImageTool(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setImageOptions(const tlr::render::ImageOptions&);

    Q_SIGNALS:
        void imageOptionsChanged(const tlr::render::ImageOptions&);

    private Q_SLOTS:
        void _yuvRangeCallback(tlr::render::YUVRange);
        void _channelsCallback(tlr::render::Channels);
        void _alphaBlendCallback(tlr::render::AlphaBlend);
        void _colorEnabledCallback(bool);
        void _colorCallback(const tlr::render::Color&);
        void _currentItemCallback(int);

    private:
        void _optionsUpdate();

        render::ImageOptions _imageOptions;

        YUVRangeWidget* _yuvRangeWidget = nullptr;
        ChannelsWidget* _channelsWidget = nullptr;
        AlphaBlendWidget* _alphaBlendWidget = nullptr;
        ColorWidget* _colorWidget = nullptr;
    };
}
