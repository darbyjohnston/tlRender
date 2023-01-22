// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/FilmstripWidget.h>

#include <tlTimeline/Timeline.h>

#include <QMainWindow>

namespace tl
{
    namespace examples
    {
        namespace filmstrip_qtwidget
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

                std::shared_ptr<system::Context> _context;
                std::string _input;
                std::shared_ptr<timeline::Timeline> _timeline;
                qt::TimelineThumbnailProvider* _thumbnailProvider = nullptr;
                qtwidget::FilmstripWidget* _filmstripWidget = nullptr;
            };
        }
    }
}
