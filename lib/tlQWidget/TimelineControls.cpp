// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/TimelineControls.h>

#include <tlQWidget/TimeLabel.h>
#include <tlQWidget/TimeSpinBox.h>

#include <tlCore/StringFormat.h>

#include <QButtonGroup>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMap>
#include <QMenu>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace tl
{
    namespace qwidget
    {
        namespace
        {
            const size_t sliderSteps = 100;
        }

        struct TimelineControls::Private
        {
            qt::TimelinePlayer* timelinePlayer = nullptr;
            QMap<QString, QToolButton*> playbackButtons;
            QButtonGroup* playbackButtonGroup = nullptr;
            QMap<QAbstractButton*, timeline::Playback> buttonToPlayback;
            QMap<timeline::Playback, QAbstractButton*> playbackToButton;
            QMap<QString, QToolButton*> timeActionButtons;
            QButtonGroup* timeActionButtonGroup = nullptr;
            QMap<QAbstractButton*, timeline::TimeAction> buttonToTimeAction;
            TimeSpinBox* currentTimeSpinBox = nullptr;
            TimeLabel* durationLabel = nullptr;
            QList<double> speeds;
            QMap<QAction*, double> actionToSpeed;
            QMenu* speedMenu = nullptr;
            QToolButton* speedButton = nullptr;
            QDoubleSpinBox* speedSpinBox = nullptr;
            QToolButton* muteButton = nullptr;
            QSlider* volumeSlider = nullptr;
        };

        TimelineControls::TimelineControls(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.playbackButtons["Stop"] = new QToolButton;
            p.playbackButtons["Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            p.playbackButtons["Stop"]->setToolTip(tr("Stop playback"));
            p.playbackButtons["Forward"] = new QToolButton;
            p.playbackButtons["Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            p.playbackButtons["Forward"]->setToolTip(tr("Forward playback"));
            p.playbackButtons["Reverse"] = new QToolButton;
            p.playbackButtons["Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            p.playbackButtons["Reverse"]->setToolTip(tr("Reverse playback"));
            for (auto i : p.playbackButtons)
            {
                i->setCheckable(true);
                i->setAutoRaise(true);
            }
            p.playbackButtonGroup = new QButtonGroup(this);
            p.playbackButtonGroup->setExclusive(true);
            p.playbackButtonGroup->addButton(p.playbackButtons["Stop"]);
            p.playbackButtonGroup->addButton(p.playbackButtons["Forward"]);
            p.playbackButtonGroup->addButton(p.playbackButtons["Reverse"]);
            p.buttonToPlayback[p.playbackButtons["Stop"]] = timeline::Playback::Stop;
            p.buttonToPlayback[p.playbackButtons["Forward"]] = timeline::Playback::Forward;
            p.buttonToPlayback[p.playbackButtons["Reverse"]] = timeline::Playback::Reverse;
            p.playbackToButton[timeline::Playback::Stop] = p.playbackButtons["Stop"];
            p.playbackToButton[timeline::Playback::Forward] = p.playbackButtons["Forward"];
            p.playbackToButton[timeline::Playback::Reverse] = p.playbackButtons["Reverse"];

            p.timeActionButtons["Start"] = new QToolButton;
            p.timeActionButtons["Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            p.timeActionButtons["Start"]->setToolTip(tr("Go to the start time"));
            p.timeActionButtons["End"] = new QToolButton;
            p.timeActionButtons["End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            p.timeActionButtons["End"]->setToolTip(tr("Go to the end time"));
            p.timeActionButtons["FramePrev"] = new QToolButton;
            p.timeActionButtons["FramePrev"]->setAutoRepeat(true);
            p.timeActionButtons["FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
            p.timeActionButtons["FramePrev"]->setToolTip(tr("Go to the previous frame"));
            p.timeActionButtons["FrameNext"] = new QToolButton;
            p.timeActionButtons["FrameNext"]->setAutoRepeat(true);
            p.timeActionButtons["FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
            p.timeActionButtons["FrameNext"]->setToolTip(tr("Go to the next frame"));
            for (auto i : p.timeActionButtons)
            {
                i->setAutoRaise(true);
            }
            p.timeActionButtonGroup = new QButtonGroup(this);
            p.timeActionButtonGroup->addButton(p.timeActionButtons["Start"]);
            p.timeActionButtonGroup->addButton(p.timeActionButtons["End"]);
            p.timeActionButtonGroup->addButton(p.timeActionButtons["FramePrev"]);
            p.timeActionButtonGroup->addButton(p.timeActionButtons["FrameNext"]);
            p.buttonToTimeAction[p.timeActionButtons["Start"]] = timeline::TimeAction::Start;
            p.buttonToTimeAction[p.timeActionButtons["End"]] = timeline::TimeAction::End;
            p.buttonToTimeAction[p.timeActionButtons["FramePrev"]] = timeline::TimeAction::FramePrev;
            p.buttonToTimeAction[p.timeActionButtons["FrameNext"]] = timeline::TimeAction::FrameNext;

            p.currentTimeSpinBox = new TimeSpinBox;
            p.currentTimeSpinBox->setToolTip(tr("Current time"));

            p.speeds.append(1.0);
            p.speeds.append(3.0);
            p.speeds.append(6.0);
            p.speeds.append(9.0);
            p.speeds.append(12.0);
            p.speeds.append(16.0);
            p.speeds.append(18.0);
            p.speeds.append(23.98);
            p.speeds.append(24.0);
            p.speeds.append(29.97);
            p.speeds.append(30.0);
            p.speeds.append(48.0);
            p.speeds.append(59.94);
            p.speeds.append(60.0);
            p.speeds.append(120.0);
            p.speedMenu = new QMenu(this);
            p.speedButton = new QToolButton;
            p.speedButton->setIcon(QIcon(":/Icons/Speed.svg"));
            p.speedButton->setMenu(p.speedMenu);
            p.speedButton->setPopupMode(QToolButton::InstantPopup);
            p.speedButton->setAutoRaise(true);
            p.speedButton->setToolTip(tr("Timeline speed menu"));
            p.speedSpinBox = new QDoubleSpinBox;
            p.speedSpinBox->setRange(0.0, 120.0);
            p.speedSpinBox->setSingleStep(1.0);
            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            p.speedSpinBox->setFont(fixedFont);
            p.speedSpinBox->setToolTip(tr("Timeline speed (frames per second)"));

            p.durationLabel = new TimeLabel;
            p.durationLabel->setToolTip(tr("Timeline duration"));

            p.muteButton = new QToolButton;
            p.muteButton->setCheckable(true);
            QIcon muteIcon;
            muteIcon.addFile(":/Icons/Volume.svg", QSize(20, 20), QIcon::Normal, QIcon::Off);
            muteIcon.addFile(":/Icons/Mute.svg", QSize(20, 20), QIcon::Normal, QIcon::On);
            p.muteButton->setIcon(muteIcon);
            p.muteButton->setAutoRaise(true);
            p.muteButton->setToolTip(tr("Mute the audio"));

            p.volumeSlider = new QSlider(Qt::Horizontal);
            p.volumeSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            p.volumeSlider->setToolTip(tr("Audio volume"));

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            auto hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addWidget(p.playbackButtons["Reverse"]);
            hLayout->addWidget(p.playbackButtons["Stop"]);
            hLayout->addWidget(p.playbackButtons["Forward"]);
            layout->addLayout(hLayout);
            hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addWidget(p.timeActionButtons["Start"]);
            hLayout->addWidget(p.timeActionButtons["FramePrev"]);
            hLayout->addWidget(p.timeActionButtons["FrameNext"]);
            hLayout->addWidget(p.timeActionButtons["End"]);
            layout->addLayout(hLayout);
            layout->addWidget(p.currentTimeSpinBox);
            layout->addWidget(p.durationLabel);
            hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addWidget(p.speedButton);
            hLayout->addWidget(p.speedSpinBox);
            layout->addLayout(hLayout);
            layout->addStretch();
            hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addWidget(p.muteButton);
            hLayout->addWidget(p.volumeSlider);
            layout->addLayout(hLayout);
            setLayout(layout);

            _playbackUpdate();
            _widgetUpdate();

            connect(
                p.playbackButtonGroup,
                SIGNAL(buttonClicked(QAbstractButton*)),
                SLOT(_playbackCallback(QAbstractButton*)));

            connect(
                p.timeActionButtonGroup,
                SIGNAL(buttonClicked(QAbstractButton*)),
                SLOT(_timeActionCallback(QAbstractButton*)));

            connect(
                p.currentTimeSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_currentTimeCallback(const otime::RationalTime&)));

            connect(
                p.speedSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(_speedCallback(double)));
            connect(
                p.speedButton,
                SIGNAL(triggered(QAction*)),
                SLOT(_speedCallback(QAction*)));

            connect(
                p.volumeSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_volumeCallback(int)));
            connect(
                p.muteButton,
                SIGNAL(toggled(bool)),
                SLOT(_muteCallback(bool)));
        }
        
        TimelineControls::~TimelineControls()
        {}

        void TimelineControls::setTimeObject(qt::TimeObject* timeObject)
        {
            TLRENDER_P();
            p.currentTimeSpinBox->setTimeObject(timeObject);
            p.durationLabel->setTimeObject(timeObject);
        }

        void TimelineControls::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
        {
            TLRENDER_P();
            if (timelinePlayer == p.timelinePlayer)
                return;
            if (p.timelinePlayer)
            {
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(speedChanged(double)),
                    this,
                    SLOT(_speedCallback2(double)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(volumeChanged(float)),
                    this,
                    SLOT(_volumeCallback2(float)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(muteChanged(bool)),
                    this,
                    SLOT(_muteCallback2(bool)));
            }
            p.timelinePlayer = timelinePlayer;
            if (p.timelinePlayer)
            {
                connect(
                    p.timelinePlayer,
                    SIGNAL(speedChanged(double)),
                    SLOT(_speedCallback2(double)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(volumeChanged(float)),
                    SLOT(_volumeCallback2(float)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(muteChanged(bool)),
                    SLOT(_muteCallback2(bool)));
            }
            _playbackUpdate();
            _widgetUpdate();
        }

        const QList<double>& TimelineControls::speeds() const
        {
            return _p->speeds;
        }

        void TimelineControls::setSpeeds(const QList<double>& value)
        {
            TLRENDER_P();
            if (p.speeds == value)
                return;
            p.speeds = value;
            _widgetUpdate();
            Q_EMIT speedsChanged(p.speeds);
        }

        void TimelineControls::focusCurrentFrame()
        {
            _p->currentTimeSpinBox->setFocus(Qt::OtherFocusReason);
            _p->currentTimeSpinBox->selectAll();
        }

        void TimelineControls::_speedCallback(double value)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setSpeed(value);
            }
        }

        void TimelineControls::_speedCallback2(double)
        {
            _widgetUpdate();
        }

        void TimelineControls::_speedCallback(QAction* action)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                auto i = p.actionToSpeed.find(action);
                if (i != p.actionToSpeed.end())
                {
                    p.timelinePlayer->setSpeed(i.value());
                }
            }
        }

        void TimelineControls::_playbackCallback(QAbstractButton* button)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                const auto i = p.buttonToPlayback.find(button);
                if (i != p.buttonToPlayback.end())
                {
                    p.timelinePlayer->setPlayback(i.value());
                    _playbackUpdate();
                }
            }
        }

        void TimelineControls::_playbackCallback(tl::timeline::Playback value)
        {
            _playbackUpdate();
        }

        void TimelineControls::_timeActionCallback(QAbstractButton* button)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                const auto i = p.buttonToTimeAction.find(button);
                if (i != p.buttonToTimeAction.end())
                {
                    p.timelinePlayer->timeAction(i.value());
                }
            }
        }

        void TimelineControls::_currentTimeCallback(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setPlayback(timeline::Playback::Stop);
                p.timelinePlayer->seek(value);
            }
        }

        void TimelineControls::_currentTimeCallback2(const otime::RationalTime& value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.currentTimeSpinBox);
            p.currentTimeSpinBox->setValue(value);
        }

        void TimelineControls::_volumeCallback(int value)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setVolume(value / static_cast<float>(sliderSteps));
            }
        }

        void TimelineControls::_volumeCallback2(float value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.volumeSlider);
            p.volumeSlider->setValue(value * sliderSteps);
        }

        void TimelineControls::_muteCallback(bool value)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setMute(value);
            }
        }

        void TimelineControls::_muteCallback2(bool value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.muteButton);
            p.muteButton->setChecked(value);
        }

        void TimelineControls::_playbackUpdate()
        {
            TLRENDER_P();
            double speed = 24.0;
            timeline::Playback playback = timeline::Playback::Stop;
            if (p.timelinePlayer)
            {
                speed = p.timelinePlayer->speed();
                playback = p.timelinePlayer->playback();
            }
            {
                const QSignalBlocker blocker(p.playbackButtonGroup);
                p.playbackToButton[playback]->setChecked(true);
            }
        }

        void TimelineControls::_widgetUpdate()
        {
            TLRENDER_P();

            p.actionToSpeed.clear();
            p.speedMenu->clear();
            for (const auto& i : p.speeds)
            {
                auto action = new QAction(this);
                action->setText(QString("%1").arg(i, 0, 'f', 2));
                p.actionToSpeed[action] = i;
                p.speedMenu->addAction(action);
            }

            for (const auto& button : p.playbackButtons)
            {
                button->setEnabled(p.timelinePlayer);
            }
            for (const auto& button : p.timeActionButtons)
            {
                button->setEnabled(p.timelinePlayer);
            }
            p.currentTimeSpinBox->setEnabled(p.timelinePlayer);
            p.speedSpinBox->setEnabled(p.timelinePlayer);
            p.speedButton->setEnabled(p.timelinePlayer);
            p.volumeSlider->setEnabled(p.timelinePlayer);
            p.muteButton->setEnabled(p.timelinePlayer);

            if (p.timelinePlayer)
            {
                {
                    const QSignalBlocker blocker(p.playbackButtonGroup);
                    p.playbackToButton[p.timelinePlayer->playback()]->setChecked(true);
                }
                {
                    const QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(p.timelinePlayer->currentTime());
                }

                const auto& duration = p.timelinePlayer->duration();
                p.durationLabel->setValue(duration);

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(p.timelinePlayer->speed());
                }
                const double defaultSpeed = p.timelinePlayer->defaultSpeed();
                auto defaultSpeedAction = new QAction(this);
                defaultSpeedAction->setText(QString(tr("Default: %1")).arg(defaultSpeed, 0, 'f', 2));
                p.actionToSpeed[defaultSpeedAction] = defaultSpeed;
                p.speedMenu->addSeparator();
                p.speedMenu->addAction(defaultSpeedAction);

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(p.timelinePlayer->volume() * sliderSteps);
                }
                {
                    QSignalBlocker blocker(p.muteButton);
                    p.muteButton->setChecked(p.timelinePlayer->isMuted());
                }
            }
            else
            {
                {
                    const QSignalBlocker blocker(p.playbackButtonGroup);
                    for (const auto& button : p.playbackButtons)
                    {
                        button->setChecked(false);
                    }
                }

                {
                    const QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(time::invalidTime);
                }

                p.durationLabel->setValue(time::invalidTime);

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(0.0);
                }

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(0);
                }
                {
                    QSignalBlocker blocker(p.muteButton);
                    p.muteButton->setChecked(false);
                }
            }
        }
    }
}
