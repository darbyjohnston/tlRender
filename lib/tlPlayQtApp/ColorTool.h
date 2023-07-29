// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <tlTimeline/IRender.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! Configuration widget.
        class ConfigWidget : public QWidget
        {
            Q_OBJECT

        public:
            ConfigWidget(App*, QWidget* parent = nullptr);

            ~ConfigWidget() override;

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! LUT widget.
        class LUTWidget : public QWidget
        {
            Q_OBJECT

        public:
            LUTWidget(App*, QWidget* parent = nullptr);

            ~LUTWidget() override;

        public Q_SLOTS:
            void setLUTOptions(const tl::timeline::LUTOptions&);

        Q_SIGNALS:
            void lutOptionsChanged(const tl::timeline::LUTOptions&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color controls widget.
        class ColorControlsWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorControlsWidget(QWidget* parent = nullptr);

            ~ColorControlsWidget() override;

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

        //! EXR display widget.
        class EXRDisplayWidget : public QWidget
        {
            Q_OBJECT

        public:
            EXRDisplayWidget(QWidget* parent = nullptr);

            ~EXRDisplayWidget() override;

        public Q_SLOTS:
            void setEXRDisplayEnabled(bool);
            void setEXRDisplay(const tl::timeline::EXRDisplay&);

        Q_SIGNALS:
            void exrDisplayEnabledChanged(bool);
            void exrDisplayChanged(const tl::timeline::EXRDisplay&);

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
        class ColorTool : public IToolWidget
        {
            Q_OBJECT

        public:
            ColorTool(App*, QWidget* parent = nullptr);

            ~ColorTool() override;

        public Q_SLOTS:
            void setLUTOptions(const tl::timeline::LUTOptions&);
            void setDisplayOptions(const tl::timeline::DisplayOptions&);

        Q_SIGNALS:
            void lutOptionsChanged(const tl::timeline::LUTOptions&);
            void displayOptionsChanged(const tl::timeline::DisplayOptions&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color tool dock widget.
        class ColorDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            ColorDockWidget(
                ColorTool*,
                QWidget* parent = nullptr);
        };
    }
}
