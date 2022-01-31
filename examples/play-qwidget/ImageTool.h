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
        void setColor(const tlr::render::Color&);

    Q_SIGNALS:
        void colorEnabledChanged(bool);
        void colorChanged(const tlr::render::Color&);

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
        render::Color _color;

        QCheckBox* _colorEnabledCheckBox = nullptr;
        ColorSlidersWidget* _addSliders = nullptr;
        ColorSlidersWidget* _brightnessSliders = nullptr;
        ColorSlidersWidget* _contrastSliders = nullptr;
        ColorSlidersWidget* _saturationSliders = nullptr;
        ColorSliderWidget* _tintSlider = nullptr;
        QCheckBox* _invertCheckBox = nullptr;
    };

    //! Levels widget.
    class LevelsWidget : public QWidget
    {
        Q_OBJECT

    public:
        LevelsWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setLevelsEnabled(bool);
        void setLevels(const tlr::render::Levels&);

    Q_SIGNALS:
        void levelsEnabledChanged(bool);
        void levelsChanged(const tlr::render::Levels&);

    private Q_SLOTS:
        void _levelsEnabledCallback(bool);
        void _inLowCallback(float);
        void _inHighCallback(float);
        void _gammaCallback(float);
        void _outLowCallback(float);
        void _outHighCallback(float);

    private:
        void _widgetUpdate();

        bool _levelsEnabled = false;
        render::Levels _levels;

        QCheckBox* _levelsEnabledCheckBox = nullptr;
        ColorSliderWidget* _inLowSlider = nullptr;
        ColorSliderWidget* _inHighSlider = nullptr;
        ColorSliderWidget* _gammaSlider = nullptr;
        ColorSliderWidget* _outLowSlider = nullptr;
        ColorSliderWidget* _outHighSlider = nullptr;
    };

    //! Exposure widget.
    class ExposureWidget : public QWidget
    {
        Q_OBJECT

    public:
        ExposureWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setExposureEnabled(bool);
        void setExposure(const tlr::render::Exposure&);

    Q_SIGNALS:
        void exposureEnabledChanged(bool);
        void exposureChanged(const tlr::render::Exposure&);

    private Q_SLOTS:
        void _exposureEnabledCallback(bool);
        void _exposureCallback(float);
        void _defogCallback(float);
        void _kneeLowCallback(float);
        void _kneeHighCallback(float);

    private:
        void _widgetUpdate();

        bool _exposureEnabled = false;
        render::Exposure _exposure;

        QCheckBox* _exposureEnabledCheckBox = nullptr;
        ColorSliderWidget* _exposureSlider = nullptr;
        ColorSliderWidget* _defogSlider = nullptr;
        ColorSliderWidget* _kneeLowSlider = nullptr;
        ColorSliderWidget* _kneeHighSlider = nullptr;
    };

    //! Soft clip widget.
    class SoftClipWidget : public QWidget
    {
        Q_OBJECT

    public:
        SoftClipWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setSoftClipEnabled(bool);
        void setSoftClip(float);

    Q_SIGNALS:
        void softClipEnabledChanged(bool);
        void softClipChanged(float);

    private Q_SLOTS:
        void _softClipEnabledCallback(bool);
        void _softClipCallback(float);

    private:
        void _widgetUpdate();

        bool _softClipEnabled = false;
        float _softClip = 0.F;

        QCheckBox* _softClipEnabledCheckBox = nullptr;
        ColorSliderWidget* _softClipSlider = nullptr;
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
        void _levelsEnabledCallback(bool);
        void _levelsCallback(const tlr::render::Levels&);
        void _exposureEnabledCallback(bool);
        void _exposureCallback(const tlr::render::Exposure&);
        void _softClipEnabledCallback(bool);
        void _softClipCallback(float);
        void _currentItemCallback(int);

    private:
        void _optionsUpdate();

        render::ImageOptions _imageOptions;

        YUVRangeWidget* _yuvRangeWidget = nullptr;
        ChannelsWidget* _channelsWidget = nullptr;
        AlphaBlendWidget* _alphaBlendWidget = nullptr;
        ColorWidget* _colorWidget = nullptr;
        LevelsWidget* _levelsWidget = nullptr;
        ExposureWidget* _exposureWidget = nullptr;
        SoftClipWidget* _softClipWidget = nullptr;
    };
}
