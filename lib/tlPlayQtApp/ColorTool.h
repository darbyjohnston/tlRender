// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlPlay/OCIOModel.h>

#include <tlQt/MetaTypes.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! OpenColorIO widget.
        class OCIOWidget : public QWidget
        {
            Q_OBJECT

        public:
            OCIOWidget(App*, QWidget* parent = nullptr);

            virtual ~OCIOWidget();

        private:
            void _widgetUpdate(const play::OCIOModelData&);

            DTK_PRIVATE();
        };

        //! LUT widget.
        class LUTWidget : public QWidget
        {
            Q_OBJECT

        public:
            LUTWidget(App*, QWidget* parent = nullptr);

            virtual ~LUTWidget();

        private:
            void _widgetUpdate(const tl::timeline::LUTOptions&);

            DTK_PRIVATE();
        };

        //! Color widget.
        class ColorWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorWidget(App*, QWidget* parent = nullptr);

            virtual ~ColorWidget();

        private:
            void _widgetUpdate(const timeline::DisplayOptions&);

            DTK_PRIVATE();
        };

        //! Levels widget.
        class LevelsWidget : public QWidget
        {
            Q_OBJECT

        public:
            LevelsWidget(App*, QWidget* parent = nullptr);

            virtual ~LevelsWidget();

        private:
            void _widgetUpdate(const timeline::DisplayOptions&);

            DTK_PRIVATE();
        };

        //! EXR display widget.
        class EXRDisplayWidget : public QWidget
        {
            Q_OBJECT

        public:
            EXRDisplayWidget(App*, QWidget* parent = nullptr);

            virtual ~EXRDisplayWidget();

        private:
            void _widgetUpdate(const timeline::DisplayOptions&);

            DTK_PRIVATE();
        };

        //! Soft clip widget.
        class SoftClipWidget : public QWidget
        {
            Q_OBJECT

        public:
            SoftClipWidget(App*, QWidget* parent = nullptr);

            virtual ~SoftClipWidget();

        private:
            void _widgetUpdate(const timeline::DisplayOptions&);

            DTK_PRIVATE();
        };

        //! Color tool.
        class ColorTool : public IToolWidget
        {
            Q_OBJECT

        public:
            ColorTool(App*, QWidget* parent = nullptr);

            virtual ~ColorTool();

        private:
            DTK_PRIVATE();
        };

        //! Color tool dock widget.
        class ColorDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            ColorDockWidget(ColorTool*, QWidget* parent = nullptr);
        };
    }
}
