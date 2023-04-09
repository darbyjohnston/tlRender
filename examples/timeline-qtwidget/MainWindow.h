// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/FloatSlider.h>
#include <tlQtWidget/IntSlider.h>
#include <tlQtWidget/TimelineViewport.h>

#include <tlTimeline/TimelinePlayer.h>

#include <QComboBox>
#include <QMainWindow>

#include "TimelineItem.h"
#include "TimelineScrollArea.h"
#include "TimelineWidget.h"

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Main window.
            class MainWindow : public QMainWindow
            {
                Q_OBJECT

            public:
                MainWindow(
                    const std::string& input,
                    const std::shared_ptr<system::Context>&,
                    QWidget* parent = nullptr);

            protected:
                void dragEnterEvent(QDragEnterEvent*) override;
                void dragMoveEvent(QDragMoveEvent*) override;
                void dragLeaveEvent(QDragLeaveEvent*) override;
                void dropEvent(QDropEvent*) override;

            private:
                void _open(const std::string&);
                
                std::weak_ptr<system::Context> _context;
                qt::TimelinePlayer* _timelinePlayer = nullptr;
                ItemOptions _itemOptions;
                qtwidget::TimelineViewport* _timelineViewport = nullptr;
                TimelineScrollArea* _timelineScrollArea = nullptr;
                TimelineWidget* _timelineWidget = nullptr;
                QDockWidget* _timelineDockWidget = nullptr;
                QComboBox* _timeUnitsComboBox = nullptr;
                qtwidget::FloatSlider* _scaleSlider = nullptr;
                qtwidget::IntSlider* _thumbnailHeightSlider = nullptr;
                qtwidget::IntSlider* _waveformHeightSlider = nullptr;
                QDockWidget* _viewDockWidget = nullptr;
            };
        }
    }
}
