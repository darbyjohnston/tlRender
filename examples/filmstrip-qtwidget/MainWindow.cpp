// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlCore/File.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMessageBox>
#include <QMimeData>

namespace tl
{
    namespace examples
    {
        namespace filmstrip_qtwidget
        {
            MainWindow::MainWindow(
                const std::string& input,
                const std::shared_ptr<system::Context>& context,
                QWidget* parent) :
                _context(context),
                QMainWindow(parent)
            {
                setAcceptDrops(true);

                _thumbnailProvider = new qt::TimelineThumbnailProvider(context, this);

                _filmstripWidget = new qtwidget::FilmstripWidget(_thumbnailProvider);
                _filmstripWidget->setRowCount(10);
                setCentralWidget(_filmstripWidget);

                if (!input.empty())
                {
                    _open(input);
                }

                resize(1280, 720);
            }

            void MainWindow::dragEnterEvent(QDragEnterEvent* event)
            {
                const QMimeData* mimeData = event->mimeData();
                if (mimeData->hasUrls())
                {
                    event->acceptProposedAction();
                }
            }

            void MainWindow::dragMoveEvent(QDragMoveEvent* event)
            {
                const QMimeData* mimeData = event->mimeData();
                if (mimeData->hasUrls())
                {
                    event->acceptProposedAction();
                }
            }

            void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
            {
                event->accept();
            }

            void MainWindow::dropEvent(QDropEvent* event)
            {
                const QMimeData* mimeData = event->mimeData();
                if (mimeData->hasUrls())
                {
                    const auto urlList = mimeData->urls();
                    for (int i = 0; i < urlList.size(); ++i)
                    {
                        _open(urlList[i].toLocalFile().toUtf8().data());
                    }
                }
            }

            void MainWindow::_open(const std::string& fileName)
            {
                try
                {
                    auto timeline = timeline::Timeline::create(fileName, _context);
                    _filmstripWidget->setTimeline(timeline);
                }
                catch (const std::exception& e)
                {
                    QMessageBox dialog;
                    dialog.setText(e.what());
                    dialog.exec();
                }
            }
        }
    }
}
