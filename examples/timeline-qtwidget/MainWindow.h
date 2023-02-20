// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "TimelineItem.h"

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
                otio::SerializableObject::Retainer<otio::Timeline> _otioTimeline;
                QGraphicsScene* _scene = nullptr;
                QGraphicsView* _view = nullptr;
                TimelineItem* _timelineItem = nullptr;
            };
        }
    }
}
