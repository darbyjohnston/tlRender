// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/ToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <tlTimeline/IRender.h>

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
            void setColor(const tl::timeline::Color&);

        Q_SIGNALS:
            void colorEnabledChanged(bool);
            void colorChanged(const tl::timeline::Color&);

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
            void setLevels(const tl::timeline::Levels&);

        Q_SIGNALS:
            void levelsEnabledChanged(bool);
            void levelsChanged(const tl::timeline::Levels&);

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
            void setExposure(const tl::timeline::Exposure&);

        Q_SIGNALS:
            void exposureEnabledChanged(bool);
            void exposureChanged(const tl::timeline::Exposure&);

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
            void setDisplayOptions(const tl::timeline::DisplayOptions&);

        Q_SIGNALS:
            void displayOptionsChanged(const tl::timeline::DisplayOptions&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
