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
        class ColorModel;

        //! Configuration widget.
        class ConfigWidget : public QWidget
        {
            Q_OBJECT

        public:
            ConfigWidget(
                const std::shared_ptr<ColorModel>& colorModel,
                QWidget* parent = nullptr);

            ~ConfigWidget() override;

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

        //! Color tool.
        class ColorTool : public ToolWidget
        {
            Q_OBJECT

        public:
            ColorTool(
                const std::shared_ptr<ColorModel>& colorModel,
                QWidget* parent = nullptr);

            ~ColorTool() override;

        public Q_SLOTS:
            void setImageOptions(const tl::render::ImageOptions&);

        Q_SIGNALS:
            void imageOptionsChanged(const tl::render::ImageOptions&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
