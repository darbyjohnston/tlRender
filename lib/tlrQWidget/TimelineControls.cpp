// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/TimelineControls.h>

#include <tlrQWidget/SpeedLabel.h>
#include <tlrQWidget/TimeLabel.h>
#include <tlrQWidget/TimeSpinBox.h>

#include <tlrCore/StringFormat.h>

#include <QButtonGroup>
#include <QComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMap>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace tlr
{
    namespace qwidget
    {
        namespace
        {
            const size_t volumeSliderSteps = 100;
        }

        struct TimelineControls::Private
        {
            qt::TimelinePlayer* timelinePlayer = nullptr;
            QMap<QString, QAbstractButton*> playbackButtons;
            QButtonGroup* playbackButtonGroup = nullptr;
            QMap<QAbstractButton*, timeline::Playback> buttonToPlayback;
            QMap<timeline::Playback, QAbstractButton*> playbackToButton;
            QMap<QString, QAbstractButton*> timeActionButtons;
            QButtonGroup* timeActionButtonGroup = nullptr;
            QMap<QAbstractButton*, timeline::TimeAction> buttonToTimeAction;
            TimeSpinBox* currentTimeSpinBox = nullptr;
            TimeSpinBox* inPointSpinBox = nullptr;
            TimeSpinBox* outPointSpinBox = nullptr;
            QMap<QString, QAbstractButton*> inOutButtons;
            TimeLabel* durationLabel = nullptr;
            SpeedLabel* speedLabel = nullptr;
            QList<double> speeds;
            QComboBox* speedComboBox = nullptr;
            QToolButton* muteButton = nullptr;
            QSlider* volumeSlider = nullptr;
        };

        TimelineControls::TimelineControls(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.playbackButtons["Stop"] = new QToolButton;
            p.playbackButtons["Stop"]->setCheckable(true);
            p.playbackButtons["Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            p.playbackButtons["Stop"]->setToolTip(tr("Stop playback"));
            p.playbackButtons["Forward"] = new QToolButton;
            p.playbackButtons["Forward"]->setCheckable(true);
            p.playbackButtons["Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            p.playbackButtons["Forward"]->setToolTip(tr("Forward playback"));
            p.playbackButtons["Reverse"] = new QToolButton;
            p.playbackButtons["Reverse"]->setCheckable(true);
            p.playbackButtons["Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            p.playbackButtons["Reverse"]->setToolTip(tr("Reverse playback"));
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

            p.inPointSpinBox = new TimeSpinBox;
            p.inPointSpinBox->setToolTip(tr("Playback in point"));

            p.outPointSpinBox = new TimeSpinBox;
            p.outPointSpinBox->setToolTip(tr("Playback out point"));

            p.inOutButtons["SetInPoint"] = new QToolButton;
            p.inOutButtons["SetInPoint"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            p.inOutButtons["SetInPoint"]->setToolTip(tr("Set the playback in point to the current frame"));
            p.inOutButtons["ResetInPoint"] = new QToolButton;
            p.inOutButtons["ResetInPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
            p.inOutButtons["ResetInPoint"]->setToolTip(tr("Reset the playback in point"));
            p.inOutButtons["SetOutPoint"] = new QToolButton;
            p.inOutButtons["SetOutPoint"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            p.inOutButtons["SetOutPoint"]->setToolTip(tr("Set the playback out point to the current frame"));
            p.inOutButtons["ResetOutPoint"] = new QToolButton;
            p.inOutButtons["ResetOutPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
            p.inOutButtons["ResetOutPoint"]->setToolTip(tr("Reset the playback out point"));

            p.speedLabel = new SpeedLabel;
            p.speedLabel->setToolTip(tr("Timeline speed (frames per second)"));

            p.speedComboBox = new QComboBox;
            p.speedComboBox->setToolTip(tr("Set the playback speed (frames per second)"));

            p.durationLabel = new TimeLabel;
            p.durationLabel->setToolTip(tr("Timeline duration"));

            p.muteButton = new QToolButton;
            p.muteButton->setCheckable(true);
            QIcon muteIcon;
            muteIcon.addFile(":/Icons/Volume.svg", QSize(20, 20), QIcon::Normal, QIcon::Off);
            muteIcon.addFile(":/Icons/Mute.svg", QSize(20, 20), QIcon::Normal, QIcon::On);
            p.muteButton->setIcon(muteIcon);
            p.muteButton->setToolTip(tr("Mute the audio"));

            p.volumeSlider = new QSlider(Qt::Horizontal);
            p.volumeSlider->setToolTip(tr("Audio volume"));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            auto hLayout = new QHBoxLayout;
            auto hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(p.playbackButtons["Reverse"]);
            hLayout2->addWidget(p.playbackButtons["Stop"]);
            hLayout2->addWidget(p.playbackButtons["Forward"]);
            hLayout->addLayout(hLayout2);
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(p.timeActionButtons["Start"]);
            hLayout2->addWidget(p.timeActionButtons["FramePrev"]);
            hLayout2->addWidget(p.timeActionButtons["FrameNext"]);
            hLayout2->addWidget(p.timeActionButtons["End"]);
            hLayout->addLayout(hLayout2);
            hLayout->addWidget(p.currentTimeSpinBox);
            hLayout->addWidget(p.inPointSpinBox);
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(p.inOutButtons["SetInPoint"]);
            hLayout2->addWidget(p.inOutButtons["ResetInPoint"]);
            hLayout->addLayout(hLayout2);
            hLayout->addStretch();
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(p.inOutButtons["ResetOutPoint"]);
            hLayout2->addWidget(p.inOutButtons["SetOutPoint"]);
            hLayout->addLayout(hLayout2);
            hLayout->addWidget(p.outPointSpinBox);
            hLayout->addWidget(p.durationLabel);
            layout->addLayout(hLayout);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.speedLabel);
            hLayout->addWidget(p.speedComboBox);
            hLayout->addStretch(1);
            hLayout->addWidget(p.muteButton);
            hLayout->addWidget(p.volumeSlider);
            layout->addLayout(hLayout);
            setLayout(layout);

            _playbackUpdate();
            _timelineUpdate();

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
                p.inPointSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_inPointCallback(const otime::RationalTime&)));
            connect(
                p.outPointSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_outPointCallback(const otime::RationalTime&)));

            connect(
                p.inOutButtons["SetInPoint"],
                SIGNAL(clicked()),
                SLOT(_inPointCallback()));
            connect(
                p.inOutButtons["ResetInPoint"],
                SIGNAL(clicked()),
                SLOT(_resetInPointCallback()));
            connect(
                p.inOutButtons["SetOutPoint"],
                SIGNAL(clicked()),
                SLOT(_outPointCallback()));
            connect(
                p.inOutButtons["ResetOutPoint"],
                SIGNAL(clicked()),
                SLOT(_resetOutPointCallback()));

            connect(
                p.speedComboBox,
                SIGNAL(activated(int)),
                SLOT(_speedCallback(int)));

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
            TLR_PRIVATE_P();
            p.currentTimeSpinBox->setTimeObject(timeObject);
            p.inPointSpinBox->setTimeObject(timeObject);
            p.outPointSpinBox->setTimeObject(timeObject);
            p.durationLabel->setTimeObject(timeObject);
        }

        void TimelineControls::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
        {
            TLR_PRIVATE_P();
            if (timelinePlayer == p.timelinePlayer)
                return;
            if (p.timelinePlayer)
            {
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(speedChanged(double)),
                    this,
                    SLOT(_speedCallback(double)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    this,
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
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
            p.speeds.clear();
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
            if (p.timelinePlayer)
            {
                const double defaultSpeed = p.timelinePlayer->defaultSpeed();
                if (!p.speeds.contains(defaultSpeed))
                {
                    p.speeds.append(defaultSpeed);
                }
                connect(
                    p.timelinePlayer,
                    SIGNAL(speedChanged(double)),
                    SLOT(_speedCallback(double)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                connect(
                    p.timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
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
            _timelineUpdate();
        }

        void TimelineControls::_speedCallback(int index)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                if (index >= 0 && index < p.speeds.size())
                {
                    p.timelinePlayer->setSpeed(p.speeds[index]);
                }
            }
        }

        void TimelineControls::_speedCallback(double)
        {
            _playbackUpdate();
        }

        void TimelineControls::_playbackCallback(QAbstractButton* button)
        {
            TLR_PRIVATE_P();
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

        void TimelineControls::_playbackCallback(tlr::timeline::Playback value)
        {
            _playbackUpdate();
        }

        void TimelineControls::_timeActionCallback(QAbstractButton* button)
        {
            TLR_PRIVATE_P();
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
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setPlayback(timeline::Playback::Stop);
                p.timelinePlayer->seek(value);
            }
        }

        void TimelineControls::_currentTimeCallback2(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            const QSignalBlocker blocker(p.currentTimeSpinBox);
            p.currentTimeSpinBox->setValue(value);
        }

        void TimelineControls::_inPointCallback(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                    value,
                    p.timelinePlayer->inOutRange().end_time_inclusive()));
            }
        }

        void TimelineControls::_inPointCallback()
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setInPoint();
            }
        }

        void TimelineControls::_resetInPointCallback()
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->resetInPoint();
            }
        }

        void TimelineControls::_outPointCallback(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                    p.timelinePlayer->inOutRange().start_time(),
                    value));
            }
        }

        void TimelineControls::_outPointCallback()
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setOutPoint();
            }
        }

        void TimelineControls::_resetOutPointCallback()
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->resetOutPoint();
            }
        }

        void TimelineControls::_inOutRangeCallback(const otime::TimeRange& value)
        {
            TLR_PRIVATE_P();
            {
                const QSignalBlocker blocker(p.inPointSpinBox);
                p.inPointSpinBox->setValue(value.start_time());
            }
            {
                const QSignalBlocker blocker(p.outPointSpinBox);
                p.outPointSpinBox->setValue(value.end_time_inclusive());
            }
        }

        void TimelineControls::_volumeCallback(int value)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setVolume(value / static_cast<float>(volumeSliderSteps));
            }
        }

        void TimelineControls::_volumeCallback2(float value)
        {
            TLR_PRIVATE_P();
            const QSignalBlocker blocker(p.volumeSlider);
            p.volumeSlider->setValue(value * volumeSliderSteps);
        }

        void TimelineControls::_muteCallback(bool value)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->setMute(value);
            }
        }

        void TimelineControls::_muteCallback2(bool value)
        {
            TLR_PRIVATE_P();
            const QSignalBlocker blocker(p.muteButton);
            p.muteButton->setChecked(value);
        }

        void TimelineControls::_playbackUpdate()
        {
            TLR_PRIVATE_P();
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
            {
                const QSignalBlocker blocker(p.speedComboBox);
                p.speedComboBox->clear();
                for (auto speed : p.speeds)
                {
                    p.speedComboBox->addItem(QString::fromStdString(string::Format("{0}").arg(speed)));
                }
                p.speedComboBox->setCurrentIndex(p.speeds.indexOf(speed));
            }
        }

        void TimelineControls::_timelineUpdate()
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                {
                    const QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.playbackToButton[p.timelinePlayer->playback()]->setChecked(true);
                }
                for (const auto& button : p.playbackButtons)
                {
                    button->setEnabled(true);
                }

                for (const auto& button : p.timeActionButtons)
                {
                    button->setEnabled(true);
                }

                {
                    const QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(p.timelinePlayer->currentTime());
                }
                p.currentTimeSpinBox->setEnabled(true);

                {
                    const QSignalBlocker blocker(p.inPointSpinBox);
                    p.inPointSpinBox->setValue(p.timelinePlayer->inOutRange().start_time());
                }
                p.inPointSpinBox->setEnabled(true);
                {
                    const QSignalBlocker blocker(p.outPointSpinBox);
                    p.outPointSpinBox->setValue(p.timelinePlayer->inOutRange().end_time_inclusive());
                }
                p.outPointSpinBox->setEnabled(true);
                for (const auto& button : p.inOutButtons)
                {
                    button->setEnabled(true);
                }

                const auto& duration = p.timelinePlayer->duration();
                p.durationLabel->setValue(duration);

                p.speedLabel->setValue(duration);
                p.speedComboBox->setEnabled(true);

                p.volumeSlider->setValue(p.timelinePlayer->volume() * volumeSliderSteps);
                p.volumeSlider->setEnabled(true);
                p.muteButton->setChecked(p.timelinePlayer->isMuted());
                p.muteButton->setEnabled(true);
            }
            else
            {
                for (const auto& button : p.playbackButtons)
                {
                    button->setChecked(false);
                    button->setEnabled(false);
                }

                for (const auto& button : p.timeActionButtons)
                {
                    button->setEnabled(false);
                }

                p.currentTimeSpinBox->setValue(time::invalidTime);
                p.currentTimeSpinBox->setEnabled(false);

                p.inPointSpinBox->setValue(time::invalidTime);
                p.inPointSpinBox->setEnabled(false);
                p.outPointSpinBox->setValue(time::invalidTime);
                p.outPointSpinBox->setEnabled(false);
                for (const auto& button : p.inOutButtons)
                {
                    button->setEnabled(false);
                }

                p.durationLabel->setValue(time::invalidTime);

                p.speedLabel->setValue(time::invalidTime);
                p.speedComboBox->setEnabled(false);

                p.volumeSlider->setValue(volumeSliderSteps);
                p.volumeSlider->setEnabled(false);
                p.muteButton->setChecked(false);
                p.muteButton->setEnabled(false);
            }
        }
    }
}
