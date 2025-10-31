// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlQtWidget/Viewport.h>
#include <tlQtWidget/TimeLabel.h>
#include <tlQtWidget/TimeSpinBox.h>
#include <tlQtWidget/TimelineWidget.h>

#include <QMainWindow>

namespace tl
{
    namespace examples
    {
        //! Example QWidget player application.
        namespace player_qtwidget
        {
            class MainWindow : public QMainWindow
            {
                Q_OBJECT

            public:
                MainWindow(const std::shared_ptr<ftk::Context>&);

                virtual ~MainWindow();

                void setPlayer(const QSharedPointer<qt::PlayerObject>&);

            private:
                void _currentTimeUpdate(const OTIO_NS::RationalTime&);
                void _durationUpdate(const OTIO_NS::RationalTime&);
                void _playbackUpdate(timeline::Playback);

                QSharedPointer<qt::PlayerObject> _player;
                QMetaObject::Connection _currentTimeConnection;
                QMetaObject::Connection _playbackConnection;
                qtwidget::Viewport* _viewport = nullptr;
                qtwidget::TimelineWidget* _timelineWidget = nullptr;
                QAction* _playbackAction = nullptr;
                qtwidget::TimeSpinBox* _currentTimeSpinBox = nullptr;
                qtwidget::TimeLabel* _durationLabel = nullptr;
            };
        }
    }
}
