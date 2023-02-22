// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"

#include <tlCore/File.h>

#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFormLayout>
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

                _scaleSlider = new qtwidget::FloatSlider;
                _scaleSlider->setRange(math::FloatRange(10.F, 200.F));
                _thumbnailHeightSlider = new qtwidget::IntSlider;
                _thumbnailHeightSlider->setRange(math::IntRange(100, 400));
                auto formLayout = new QFormLayout;
                formLayout->addRow(tr("Scale:"), _scaleSlider);
                formLayout->addRow(tr("Thumbnail height:"), _thumbnailHeightSlider);
                auto viewWidget = new QWidget;
                viewWidget->setLayout(formLayout);
                _viewDockWidget = new QDockWidget(tr("View"));
                _viewDockWidget->setWidget(viewWidget);
                addDockWidget(Qt::RightDockWidgetArea, _viewDockWidget);

                _scaleSlider->setValue(100.F);
                _thumbnailHeightSlider->setValue(100);

                if (!input.empty())
                {
                    _open(input);
                }

                resize(1280, 720);

                connect(
                    _scaleSlider,
                    &qtwidget::FloatSlider::valueChanged,
                    [this](float value)
                    {
                        if (_timelineItem)
                        {
                            _timelineItem->setScale(value);
                        }
                    });

                connect(
                    _thumbnailHeightSlider,
                    &qtwidget::IntSlider::valueChanged,
                    [this](int value)
                    {
                        if (_timelineItem)
                        {
                            _timelineItem->setThumbnailHeight(value);
                        }
                    });
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
                _timeline = nullptr;

                try
                {
                    if (auto context = _context.lock())
                    {
                        _timeline = timeline::Timeline::create(fileName, context);

                        ItemOptions options;
                        options.font = font();
                        const auto fm = QFontMetrics(options.font);
                        options.fontLineSize = fm.lineSpacing();
                        options.fontAscender = fm.ascent();
                        options.fontDescender = fm.descent();

                        _timelineItem = new TimelineItem(_timeline, options, context);
                        _timelineItem->layout();
                        _scene->addItem(_timelineItem);
                    }
                }
                catch (const std::string& e)
                {
                    QMessageBox dialog;
                    dialog.setText(QString::fromUtf8(e.c_str()));
                    dialog.exec();
                }
            }
        }
    }
}
