// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "TimelineWidget.h"

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

                _timelineViewport = new qtwidget::TimelineViewport(context);
                setCentralWidget(_timelineViewport);

                _timelineWidget = new TimelineWidget(context);
                _timelineScrollArea = new TimelineScrollArea;
                _timelineScrollArea->setTimelineWidget(_timelineWidget);
                _timelineDockWidget = new QDockWidget(tr("View"));
                _timelineDockWidget->setWidget(_timelineScrollArea);
                addDockWidget(Qt::BottomDockWidgetArea, _timelineDockWidget);

                _timeUnitsComboBox = new QComboBox;
                for (auto i : getTimeUnitsLabels())
                {
                    _timeUnitsComboBox->addItem(QString::fromLatin1(i.c_str()));
                }
                _scaleSlider = new qtwidget::FloatSlider;
                _scaleSlider->setRange(math::FloatRange(1.F, 1000.F));
                _thumbnailHeightSlider = new qtwidget::IntSlider;
                _thumbnailHeightSlider->setRange(math::IntRange(100, 1000));
                _waveformHeightSlider = new qtwidget::IntSlider;
                _waveformHeightSlider->setRange(math::IntRange(100, 1000));
                auto formLayout = new QFormLayout;
                formLayout->addRow(tr("Time units:"), _timeUnitsComboBox);
                formLayout->addRow(tr("Scale:"), _scaleSlider);
                formLayout->addRow(tr("Thumbnail height:"), _thumbnailHeightSlider);
                formLayout->addRow(tr("Waveform height:"), _waveformHeightSlider);
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
                    _timeUnitsComboBox,
                    QOverload<int>::of(&QComboBox::activated),
                    [this](int value)
                    {
                        _itemOptions.timeUnits = static_cast<TimeUnits>(value);
                        _timelineWidget->setItemOptions(_itemOptions);
                    });

                connect(
                    _scaleSlider,
                    &qtwidget::FloatSlider::valueChanged,
                    [this](float value)
                    {
                        _itemOptions.scale = value;
                        _timelineWidget->setItemOptions(_itemOptions);
                    });

                connect(
                    _thumbnailHeightSlider,
                    &qtwidget::IntSlider::valueChanged,
                    [this](int value)
                    {
                        _itemOptions.thumbnailHeight = value;
                        _timelineWidget->setItemOptions(_itemOptions);
                    });

                connect(
                    _waveformHeightSlider,
                    &qtwidget::IntSlider::valueChanged,
                    [this](int value)
                    {
                        _itemOptions.waveformHeight = value;
                        _timelineWidget->setItemOptions(_itemOptions);
                    });

                connect(
                    _timelineWidget,
                    &TimelineWidget::currentTimeChanged,
                    [this](const otime::RationalTime& value)
                    {
                        _timelinePlayer->seek(value);
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
                delete _timelinePlayer;
                _timelinePlayer = nullptr;

                std::shared_ptr<timeline::Timeline> timeline;
                std::vector<qt::TimelinePlayer*> timelinePlayers;
                try
                {
                    if (auto context = _context.lock())
                    {
                        timeline = timeline::Timeline::create(fileName, context);
                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = otime::RationalTime(1.0, 1.0);
                        auto timelinePlayer = timeline::TimelinePlayer::create(timeline, context, playerOptions);
                        timelinePlayers.push_back(new qt::TimelinePlayer(timelinePlayer, context));
                        _timelinePlayer = new qt::TimelinePlayer(timelinePlayer, context);
                    }
                }
                catch (const std::string& e)
                {
                    timeline.reset();
                    timelinePlayers.clear();

                    QMessageBox dialog;
                    dialog.setText(QString::fromUtf8(e.c_str()));
                    dialog.exec();
                }
                _timelineViewport->setTimelinePlayers(timelinePlayers);
                _timelineWidget->setTimeline(timeline);
            }
        }
    }
}
