// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"

#include <tlCore/File.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMessageBox>
#include <QMimeData>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            MainWindow::MainWindow(
                const std::string& input,
                const std::shared_ptr<system::Context>& context,
                QWidget* parent) :
                QMainWindow(parent),
                _context(context)
            {
                setAcceptDrops(true);

                _scene = new QGraphicsScene(this);

                _view = new QGraphicsView(this);
                _view->setScene(_scene);
                setCentralWidget(_view);

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
                _scene->clear();
                _timelineItem = nullptr;
                _otioTimeline = nullptr;

                otio::ErrorStatus errorStatus;
                _otioTimeline = dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_file(fileName, &errorStatus));
                if (otio::is_error(errorStatus))
                {
                    _otioTimeline = nullptr;
                    QMessageBox dialog;
                    dialog.setText(QString::fromUtf8(errorStatus.details.c_str()));
                    dialog.exec();
                }

                if (_otioTimeline)
                {
                    if (auto context = _context.lock())
                    {
                        ItemOptions options;
                        options.font = font();
                        const auto fm = QFontMetrics(options.font);
                        options.fontLineSize = fm.lineSpacing();
                        options.fontAscender = fm.ascent();
                        options.fontDescender = fm.descent();

                        _timelineItem = new TimelineItem(_otioTimeline.value, options);
                        _timelineItem->layout();
                        _scene->addItem(_timelineItem);
                    }
                }
            }
        }
    }
}
