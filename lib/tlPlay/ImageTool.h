// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <tlCore/IRender.h>

namespace tl
{
    namespace play
    {
        //! YUV range widget.
        class YUVRangeWidget : public QWidget
        {
            Q_OBJECT

        public:
            YUVRangeWidget(QWidget* parent = nullptr);

            ~YUVRangeWidget() override;

        public Q_SLOTS:
            void setValue(tl::render::YUVRange);

        Q_SIGNALS:
            void valueChanged(tl::render::YUVRange);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Channels widget.
        class ChannelsWidget : public QWidget
        {
            Q_OBJECT

        public:
            ChannelsWidget(QWidget* parent = nullptr);

            ~ChannelsWidget() override;

        public Q_SLOTS:
            void setValue(tl::render::Channels);

        Q_SIGNALS:
            void valueChanged(tl::render::Channels);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Alpha blend widget.
        class AlphaBlendWidget : public QWidget
        {
            Q_OBJECT

        public:
            AlphaBlendWidget(QWidget* parent = nullptr);

            ~AlphaBlendWidget() override;

        public Q_SLOTS:
            void setValue(tl::render::AlphaBlend);

        Q_SIGNALS:
            void valueChanged(tl::render::AlphaBlend);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color slider widget.
        class ColorSliderWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorSliderWidget(QWidget* parent = nullptr);

            ~ColorSliderWidget() override;

            void setRange(const math::FloatRange&);

        public Q_SLOTS:
            void setValue(float);

        Q_SIGNALS:
            void valueChanged(float);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color sliders widget.
        class ColorSlidersWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorSlidersWidget(QWidget* parent = nullptr);

            ~ColorSlidersWidget() override;

            void setRange(const math::FloatRange&);

        public Q_SLOTS:
            void setValue(const tl::math::Vector3f&);
            void setComponents(bool);

        Q_SIGNALS:
            void valueChanged(const tl::math::Vector3f&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color widget.
        class ColorWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorWidget(QWidget* parent = nullptr);

            ~ColorWidget() override;

        public Q_SLOTS:
            void setColorEnabled(bool);
            void setColor(const tl::render::Color&);

        Q_SIGNALS:
            void colorEnabledChanged(bool);
            void colorChanged(const tl::render::Color&);

        private Q_SLOTS:
            void _componentsCallback(bool);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Levels widget.
        class LevelsWidget : public QWidget
        {
            Q_OBJECT

        public:
            LevelsWidget(QWidget* parent = nullptr);

            ~LevelsWidget() override;

        public Q_SLOTS:
            void setLevelsEnabled(bool);
            void setLevels(const tl::render::Levels&);

        Q_SIGNALS:
            void levelsEnabledChanged(bool);
            void levelsChanged(const tl::render::Levels&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Exposure widget.
        class ExposureWidget : public QWidget
        {
            Q_OBJECT

        public:
            ExposureWidget(QWidget* parent = nullptr);

            ~ExposureWidget() override;

        public Q_SLOTS:
            void setExposureEnabled(bool);
            void setExposure(const tl::render::Exposure&);

        Q_SIGNALS:
            void exposureEnabledChanged(bool);
            void exposureChanged(const tl::render::Exposure&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Soft clip widget.
        class SoftClipWidget : public QWidget
        {
            Q_OBJECT

        public:
            SoftClipWidget(QWidget* parent = nullptr);

            ~SoftClipWidget() override;

        public Q_SLOTS:
            void setSoftClipEnabled(bool);
            void setSoftClip(float);

        Q_SIGNALS:
            void softClipEnabledChanged(bool);
            void softClipChanged(float);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Image tool.
        class ImageTool : public ToolWidget
        {
            Q_OBJECT

        public:
            ImageTool(QWidget* parent = nullptr);

            ~ImageTool() override;

        public Q_SLOTS:
            void setImageOptions(const tl::render::ImageOptions&);

        Q_SIGNALS:
            void imageOptionsChanged(const tl::render::ImageOptions&);

        private:
            void _optionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
