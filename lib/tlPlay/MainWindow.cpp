// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/MainWindow.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioTool.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/ColorTool.h>
#include <tlPlay/CompareTool.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/FilesTool.h>
#include <tlPlay/InfoTool.h>
#include <tlPlay/MessagesTool.h>
#include <tlPlay/SecondaryWindow.h>
#include <tlPlay/SettingsObject.h>
#include <tlPlay/SettingsTool.h>
#include <tlPlay/SystemLogTool.h>

#include <tlQWidget/TimelineSlider.h>
#include <tlQWidget/TimelineViewport.h>
#include <tlQWidget/TimelineWidget.h>
#include <tlQWidget/Util.h>

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMimeData>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>

namespace tl
{
    namespace play
    {
        namespace
        {
            const size_t errorTimeout = 5000;
        }

        struct MainWindow::Private
        {
            App* app = nullptr;

            std::vector<qt::TimelinePlayer*> timelinePlayers;
            bool floatOnTop = false;
            bool secondaryFloatOnTop = false;
            imaging::ColorConfig colorConfig;
            render::ImageOptions imageOptions;
            render::CompareOptions compareOptions;

            QMap<QString, QAction*> actions;
            QActionGroup* recentFilesActionGroup = nullptr;
            QMap<QAction*, QString> actionToRecentFile;
            QMenu* recentFilesMenu = nullptr;
            QActionGroup* compareActionGroup = nullptr;
            QActionGroup* yuvRangeActionGroup = nullptr;
            QActionGroup* channelsActionGroup = nullptr;
            QActionGroup* alphaBlendActionGroup = nullptr;
            QActionGroup* playbackActionGroup = nullptr;
            QActionGroup* loopActionGroup = nullptr;

            QComboBox* filesComboBox = nullptr;

            QComboBox* filesBComboBox = nullptr;

            qwidget::TimelineWidget* timelineWidget = nullptr;
            FilesTool* filesTool = nullptr;
            CompareTool* compareTool = nullptr;
            ColorTool* colorTool = nullptr;
            InfoTool* infoTool = nullptr;
            AudioTool* audioTool = nullptr;
            SettingsTool* settingsTool = nullptr;
            MessagesTool* messagesTool = nullptr;
            SystemLogTool* systemLogTool = nullptr;
            QLabel* infoLabel = nullptr;
            QStatusBar* statusBar = nullptr;
            SecondaryWindow* secondaryWindow = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ListObserver<render::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<render::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<imaging::ColorConfig> > colorConfigObserver;
            std::shared_ptr<observer::ValueObserver<core::LogItem> > logObserver;
        };

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.imageOptions = app->imageOptions();

            setFocusPolicy(Qt::ClickFocus);
            setAcceptDrops(true);

            p.actions["File/Open"] = new QAction(this);
            p.actions["File/Open"]->setText(tr("Open"));
            p.actions["File/Open"]->setIcon(QIcon(":/Icons/FileOpen.svg"));
            p.actions["File/Open"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
            p.actions["File/Open"]->setToolTip(tr("Open a file"));
            p.actions["File/OpenWithAudio"] = new QAction(this);
            p.actions["File/OpenWithAudio"]->setText(tr("Open With Audio"));
            p.actions["File/OpenWithAudio"]->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
            p.actions["File/OpenWithAudio"]->setToolTip(tr("Open a file with audio"));
            p.actions["File/Close"] = new QAction(this);
            p.actions["File/Close"]->setText(tr("Close"));
            p.actions["File/Close"]->setIcon(QIcon(":/Icons/FileClose.svg"));
            p.actions["File/Close"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
            p.actions["File/Close"]->setToolTip(tr("Close the current file"));
            p.actions["File/CloseAll"] = new QAction(this);
            p.actions["File/CloseAll"]->setText(tr("Close All"));
            p.actions["File/CloseAll"]->setToolTip(tr("Close all files"));
            p.actions["File/Next"] = new QAction(this);
            p.actions["File/Next"]->setText(tr("Next"));
            p.actions["File/Next"]->setIcon(QIcon(":/Icons/Next.svg"));
            p.actions["File/Next"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
            p.actions["File/Next"]->setToolTip(tr("Change to the next file"));
            p.actions["File/Prev"] = new QAction(this);
            p.actions["File/Prev"]->setText(tr("Previous"));
            p.actions["File/Prev"]->setIcon(QIcon(":/Icons/Prev.svg"));
            p.actions["File/Prev"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
            p.actions["File/Prev"]->setToolTip(tr("Change to the previous file"));
            p.actions["File/NextLayer"] = new QAction(this);
            p.actions["File/NextLayer"]->setText(tr("Next Layer"));
            p.actions["File/NextLayer"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
            p.actions["File/NextLayer"]->setToolTip(tr("Change to the next layer"));
            p.actions["File/PrevLayer"] = new QAction(this);
            p.actions["File/PrevLayer"]->setText(tr("Previous Layer"));
            p.actions["File/PrevLayer"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
            p.actions["File/PrevLayer"]->setToolTip(tr("Change to the previous layer"));
            p.recentFilesActionGroup = new QActionGroup(this);
            p.actions["File/Exit"] = new QAction(this);
            p.actions["File/Exit"]->setText(tr("Exit"));
            p.actions["File/Exit"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

            p.actions["Compare/A"] = new QAction(this);
            p.actions["Compare/A"]->setData(QVariant::fromValue<render::CompareMode>(render::CompareMode::A));
            p.actions["Compare/A"]->setCheckable(true);
            p.actions["Compare/A"]->setText(tr("Show A"));
            p.actions["Compare/A"]->setIcon(QIcon(":/Icons/CompareA.svg"));
            p.actions["Compare/A"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
            p.actions["Compare/A"]->setToolTip(tr("Show the A file"));
            p.actions["Compare/B"] = new QAction(this);
            p.actions["Compare/B"]->setData(QVariant::fromValue<render::CompareMode>(render::CompareMode::B));
            p.actions["Compare/B"]->setCheckable(true);
            p.actions["Compare/B"]->setText(tr("Show B"));
            p.actions["Compare/B"]->setIcon(QIcon(":/Icons/CompareB.svg"));
            p.actions["Compare/B"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
            p.actions["Compare/B"]->setToolTip(tr("Show the B file"));
            p.actions["Compare/Wipe"] = new QAction(this);
            p.actions["Compare/Wipe"]->setData(QVariant::fromValue<render::CompareMode>(render::CompareMode::Wipe));
            p.actions["Compare/Wipe"]->setCheckable(true);
            p.actions["Compare/Wipe"]->setText(tr("A/B Wipe"));
            p.actions["Compare/Wipe"]->setIcon(QIcon(":/Icons/CompareWipe.svg"));
            p.actions["Compare/Wipe"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
            p.actions["Compare/Wipe"]->setToolTip(tr("Wipe between the A and B files"));
            p.actions["Compare/Tile"] = new QAction(this);
            p.actions["Compare/Tile"]->setData(QVariant::fromValue<render::CompareMode>(render::CompareMode::Tile));
            p.actions["Compare/Tile"]->setCheckable(true);
            p.actions["Compare/Tile"]->setText(tr("A/B Tile"));
            p.actions["Compare/Tile"]->setIcon(QIcon(":/Icons/CompareTile.svg"));
            p.actions["Compare/Tile"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
            p.actions["Compare/Tile"]->setToolTip(tr("Tile the A and B files"));
            p.compareActionGroup = new QActionGroup(this);
            p.compareActionGroup->setExclusive(true);
            p.compareActionGroup->addAction(p.actions["Compare/A"]);
            p.compareActionGroup->addAction(p.actions["Compare/B"]);
            p.compareActionGroup->addAction(p.actions["Compare/Wipe"]);
            p.compareActionGroup->addAction(p.actions["Compare/Tile"]);
            p.actions["Compare/Next"] = new QAction(this);
            p.actions["Compare/Next"]->setText(tr("Next"));
            p.actions["Compare/Next"]->setIcon(QIcon(":/Icons/Next.svg"));
            p.actions["Compare/Next"]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_PageDown));
            p.actions["Compare/Next"]->setToolTip(tr("Change to the next file"));
            p.actions["Compare/Prev"] = new QAction(this);
            p.actions["Compare/Prev"]->setText(tr("Previous"));
            p.actions["Compare/Prev"]->setIcon(QIcon(":/Icons/Prev.svg"));
            p.actions["Compare/Prev"]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_PageUp));
            p.actions["Compare/Prev"]->setToolTip(tr("Change to the previous file"));

            p.actions["Window/Resize1280x720"] = new QAction(this);
            p.actions["Window/Resize1280x720"]->setText(tr("Resize 1280x720"));
            p.actions["Window/Resize1920x1080"] = new QAction(this);
            p.actions["Window/Resize1920x1080"]->setText(tr("Resize 1920x1080"));
            p.actions["Window/Resize1920x1080"] = new QAction(this);
            p.actions["Window/Resize1920x1080"]->setText(tr("Resize 1920x1080"));
            p.actions["Window/FullScreen"] = new QAction(this);
            p.actions["Window/FullScreen"]->setText(tr("Full Screen"));
            p.actions["Window/FullScreen"]->setShortcut(QKeySequence(Qt::Key_U));
            p.actions["Window/FloatOnTop"] = new QAction(this);
            p.actions["Window/FloatOnTop"]->setCheckable(true);
            p.actions["Window/FloatOnTop"]->setText(tr("Float On Top"));
            p.actions["Window/Secondary"] = new QAction(this);
            p.actions["Window/Secondary"]->setCheckable(true);
            p.actions["Window/Secondary"]->setText(tr("Secondary"));
            p.actions["Window/Secondary"]->setShortcut(QKeySequence(Qt::Key_Y));
            p.actions["Window/SecondaryFloatOnTop"] = new QAction(this);
            p.actions["Window/SecondaryFloatOnTop"]->setCheckable(true);
            p.actions["Window/SecondaryFloatOnTop"]->setText(tr("Secondary Float On Top"));

            p.actions["View/Frame"] = new QAction(this);
            p.actions["View/Frame"]->setText(tr("Frame"));
            p.actions["View/Frame"]->setShortcut(QKeySequence(Qt::Key_Backspace));
            p.actions["View/Zoom1To1"] = new QAction(this);
            p.actions["View/Zoom1To1"]->setText(tr("Zoom 1:1"));
            p.actions["View/Zoom1To1"]->setShortcut(QKeySequence(Qt::Key_0));
            p.actions["View/ZoomIn"] = new QAction(this);
            p.actions["View/ZoomIn"]->setText(tr("Zoom In"));
            p.actions["View/ZoomIn"]->setShortcut(QKeySequence(Qt::Key_Equal));
            p.actions["View/ZoomOut"] = new QAction(this);
            p.actions["View/ZoomOut"]->setText(tr("Zoom Out"));
            p.actions["View/ZoomOut"]->setShortcut(QKeySequence(Qt::Key_Minus));

            p.actions["Image/Channels/Red"] = new QAction(this);
            p.actions["Image/Channels/Red"]->setData(QVariant::fromValue<render::Channels>(render::Channels::Red));
            p.actions["Image/Channels/Red"]->setCheckable(true);
            p.actions["Image/Channels/Red"]->setText(tr("Red Channel"));
            p.actions["Image/Channels/Red"]->setShortcut(QKeySequence(Qt::Key_R));
            p.actions["Image/Channels/Green"] = new QAction(this);
            p.actions["Image/Channels/Green"]->setData(QVariant::fromValue<render::Channels>(render::Channels::Green));
            p.actions["Image/Channels/Green"]->setCheckable(true);
            p.actions["Image/Channels/Green"]->setText(tr("Green Channel"));
            p.actions["Image/Channels/Green"]->setShortcut(QKeySequence(Qt::Key_G));
            p.actions["Image/Channels/Blue"] = new QAction(this);
            p.actions["Image/Channels/Blue"]->setData(QVariant::fromValue<render::Channels>(render::Channels::Blue));
            p.actions["Image/Channels/Blue"]->setCheckable(true);
            p.actions["Image/Channels/Blue"]->setText(tr("Blue Channel"));
            p.actions["Image/Channels/Blue"]->setShortcut(QKeySequence(Qt::Key_B));
            p.actions["Image/Channels/Alpha"] = new QAction(this);
            p.actions["Image/Channels/Alpha"]->setData(QVariant::fromValue<render::Channels>(render::Channels::Alpha));
            p.actions["Image/Channels/Alpha"]->setCheckable(true);
            p.actions["Image/Channels/Alpha"]->setText(tr("Alpha Channel"));
            p.actions["Image/Channels/Alpha"]->setShortcut(QKeySequence(Qt::Key_A));
            p.channelsActionGroup = new QActionGroup(this);
            p.channelsActionGroup->addAction(p.actions["Image/Channels/Red"]);
            p.channelsActionGroup->addAction(p.actions["Image/Channels/Green"]);
            p.channelsActionGroup->addAction(p.actions["Image/Channels/Blue"]);
            p.channelsActionGroup->addAction(p.actions["Image/Channels/Alpha"]);
            p.actions["Image/MirrorX"] = new QAction(this);
            p.actions["Image/MirrorX"]->setText(tr("Mirror Horizontal"));
            p.actions["Image/MirrorX"]->setShortcut(QKeySequence(Qt::Key_H));
            p.actions["Image/MirrorX"]->setCheckable(true);
            p.actions["Image/MirrorY"] = new QAction(this);
            p.actions["Image/MirrorY"]->setText(tr("Mirror Vertical"));
            p.actions["Image/MirrorY"]->setShortcut(QKeySequence(Qt::Key_V));
            p.actions["Image/MirrorY"]->setCheckable(true);
            p.actions["Image/YUVRange/FromFile"] = new QAction(this);
            p.actions["Image/YUVRange/FromFile"]->setData(QVariant::fromValue<render::YUVRange>(render::YUVRange::FromFile));
            p.actions["Image/YUVRange/FromFile"]->setCheckable(true);
            p.actions["Image/YUVRange/FromFile"]->setText(tr("From File"));
            p.actions["Image/YUVRange/Full"] = new QAction(this);
            p.actions["Image/YUVRange/Full"]->setData(QVariant::fromValue<render::YUVRange>(render::YUVRange::Full));
            p.actions["Image/YUVRange/Full"]->setCheckable(true);
            p.actions["Image/YUVRange/Full"]->setText(tr("Full"));
            p.actions["Image/YUVRange/Video"] = new QAction(this);
            p.actions["Image/YUVRange/Video"]->setData(QVariant::fromValue<render::YUVRange>(render::YUVRange::Video));
            p.actions["Image/YUVRange/Video"]->setCheckable(true);
            p.actions["Image/YUVRange/Video"]->setText(tr("Video"));
            p.yuvRangeActionGroup = new QActionGroup(this);
            p.yuvRangeActionGroup->addAction(p.actions["Image/YUVRange/FromFile"]);
            p.yuvRangeActionGroup->addAction(p.actions["Image/YUVRange/Full"]);
            p.yuvRangeActionGroup->addAction(p.actions["Image/YUVRange/Video"]);
            p.actions["Image/AlphaBlend/None"] = new QAction(this);
            p.actions["Image/AlphaBlend/None"]->setData(QVariant::fromValue<render::AlphaBlend>(render::AlphaBlend::None));
            p.actions["Image/AlphaBlend/None"]->setCheckable(true);
            p.actions["Image/AlphaBlend/None"]->setText(tr("None"));
            p.actions["Image/AlphaBlend/Straight"] = new QAction(this);
            p.actions["Image/AlphaBlend/Straight"]->setData(QVariant::fromValue<render::AlphaBlend>(render::AlphaBlend::Straight));
            p.actions["Image/AlphaBlend/Straight"]->setCheckable(true);
            p.actions["Image/AlphaBlend/Straight"]->setText(tr("Straight"));
            p.actions["Image/AlphaBlend/Premultiplied"] = new QAction(this);
            p.actions["Image/AlphaBlend/Premultiplied"]->setData(QVariant::fromValue<render::AlphaBlend>(render::AlphaBlend::Premultiplied));
            p.actions["Image/AlphaBlend/Premultiplied"]->setCheckable(true);
            p.actions["Image/AlphaBlend/Premultiplied"]->setText(tr("Premultiplied"));
            p.alphaBlendActionGroup = new QActionGroup(this);
            p.alphaBlendActionGroup->addAction(p.actions["Image/AlphaBlend/None"]);
            p.alphaBlendActionGroup->addAction(p.actions["Image/AlphaBlend/Straight"]);
            p.alphaBlendActionGroup->addAction(p.actions["Image/AlphaBlend/Premultiplied"]);

            p.actions["Playback/Stop"] = new QAction(this);
            p.actions["Playback/Stop"]->setData(QVariant::fromValue< timeline::Playback>(timeline::Playback::Stop));
            p.actions["Playback/Stop"]->setCheckable(true);
            p.actions["Playback/Stop"]->setText(tr("Stop Playback"));
            p.actions["Playback/Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            p.actions["Playback/Stop"]->setShortcut(QKeySequence(Qt::Key_K));
            p.actions["Playback/Forward"] = new QAction(this);
            p.actions["Playback/Forward"]->setData(QVariant::fromValue< timeline::Playback>(timeline::Playback::Forward));
            p.actions["Playback/Forward"]->setCheckable(true);
            p.actions["Playback/Forward"]->setText(tr("Forward Playback"));
            p.actions["Playback/Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            p.actions["Playback/Forward"]->setShortcut(QKeySequence(Qt::Key_L));
            p.actions["Playback/Reverse"] = new QAction(this);
            p.actions["Playback/Reverse"]->setData(QVariant::fromValue< timeline::Playback>(timeline::Playback::Reverse));
            p.actions["Playback/Reverse"]->setCheckable(true);
            p.actions["Playback/Reverse"]->setText(tr("Reverse Playback"));
            p.actions["Playback/Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            p.actions["Playback/Reverse"]->setShortcut(QKeySequence(Qt::Key_J));
            p.playbackActionGroup = new QActionGroup(this);
            p.playbackActionGroup->setExclusive(true);
            p.playbackActionGroup->addAction(p.actions["Playback/Stop"]);
            p.playbackActionGroup->addAction(p.actions["Playback/Forward"]);
            p.playbackActionGroup->addAction(p.actions["Playback/Reverse"]);
            p.actions["Playback/Toggle"] = new QAction(this);
            p.actions["Playback/Toggle"]->setText(tr("Toggle Playback"));
            p.actions["Playback/Toggle"]->setShortcut(QKeySequence(Qt::Key_Space));

            p.actions["Playback/Loop"] = new QAction(this);
            p.actions["Playback/Loop"]->setData(QVariant::fromValue<timeline::Loop>(timeline::Loop::Loop));
            p.actions["Playback/Loop"]->setCheckable(true);
            p.actions["Playback/Loop"]->setText(tr("Loop Playback"));
            p.actions["Playback/Once"] = new QAction(this);
            p.actions["Playback/Once"]->setData(QVariant::fromValue<timeline::Loop>(timeline::Loop::Once));
            p.actions["Playback/Once"]->setCheckable(true);
            p.actions["Playback/Once"]->setText(tr("Playback Once"));
            p.actions["Playback/PingPong"] = new QAction(this);
            p.actions["Playback/PingPong"]->setData(QVariant::fromValue<timeline::Loop>(timeline::Loop::PingPong));
            p.actions["Playback/PingPong"]->setCheckable(true);
            p.actions["Playback/PingPong"]->setText(tr("Ping-Pong Playback"));
            p.loopActionGroup = new QActionGroup(this);
            p.loopActionGroup->setExclusive(true);
            p.loopActionGroup->addAction(p.actions["Playback/Loop"]);
            p.loopActionGroup->addAction(p.actions["Playback/Once"]);
            p.loopActionGroup->addAction(p.actions["Playback/PingPong"]);

            p.actions["Playback/Start"] = new QAction(this);
            p.actions["Playback/Start"]->setText(tr("Go To Start"));
            p.actions["Playback/Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            p.actions["Playback/Start"]->setShortcut(QKeySequence(Qt::Key_Home));
            p.actions["Playback/End"] = new QAction(this);
            p.actions["Playback/End"]->setText(tr("Go To End"));
            p.actions["Playback/End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            p.actions["Playback/End"]->setShortcut(QKeySequence(Qt::Key_End));
            p.actions["Playback/FramePrev"] = new QAction(this);
            p.actions["Playback/FramePrev"]->setText(tr("Previous Frame"));
            p.actions["Playback/FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
            p.actions["Playback/FramePrev"]->setShortcut(QKeySequence(Qt::Key_Left));
            p.actions["Playback/FramePrevX10"] = new QAction(this);
            p.actions["Playback/FramePrevX10"]->setText(tr("Previous Frame X10"));
            p.actions["Playback/FramePrevX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
            p.actions["Playback/FramePrevX100"] = new QAction(this);
            p.actions["Playback/FramePrevX100"]->setText(tr("Previous Frame X100"));
            p.actions["Playback/FramePrevX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
            p.actions["Playback/FrameNext"] = new QAction(this);
            p.actions["Playback/FrameNext"]->setText(tr("Next Frame"));
            p.actions["Playback/FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
            p.actions["Playback/FrameNext"]->setShortcut(QKeySequence(Qt::Key_Right));
            p.actions["Playback/FrameNextX10"] = new QAction(this);
            p.actions["Playback/FrameNextX10"]->setText(tr("Next Frame X10"));
            p.actions["Playback/FrameNextX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
            p.actions["Playback/FrameNextX100"] = new QAction(this);
            p.actions["Playback/FrameNextX100"]->setText(tr("Next Frame X100"));
            p.actions["Playback/FrameNextX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));

            p.actions["Playback/SetInPoint"] = new QAction(this);
            p.actions["Playback/SetInPoint"]->setText(tr("Set In Point"));
            p.actions["Playback/SetInPoint"]->setShortcut(QKeySequence(Qt::Key_I));
            p.actions["Playback/ResetInPoint"] = new QAction(this);
            p.actions["Playback/ResetInPoint"]->setText(tr("Reset In Point"));
            p.actions["Playback/ResetInPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
            p.actions["Playback/SetOutPoint"] = new QAction(this);
            p.actions["Playback/SetOutPoint"]->setText(tr("Set Out Point"));
            p.actions["Playback/SetOutPoint"]->setShortcut(QKeySequence(Qt::Key_O));
            p.actions["Playback/ResetOutPoint"] = new QAction(this);
            p.actions["Playback/ResetOutPoint"]->setText(tr("Reset Out Point"));
            p.actions["Playback/ResetOutPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_O));

            p.actions["Playback/FocusCurrentFrame"] = new QAction(this);
            p.actions["Playback/FocusCurrentFrame"]->setText(tr("Focus Current Frame"));
            p.actions["Playback/FocusCurrentFrame"]->setShortcut(QKeySequence(Qt::Key_F));

            p.actions["Audio/IncreaseVolume"] = new QAction(this);
            p.actions["Audio/IncreaseVolume"]->setText(tr("Increase Volume"));
            p.actions["Audio/IncreaseVolume"]->setShortcut(QKeySequence(Qt::Key_Period));
            p.actions["Audio/DecreaseVolume"] = new QAction(this);
            p.actions["Audio/DecreaseVolume"]->setText(tr("Decrease Volume"));
            p.actions["Audio/DecreaseVolume"]->setShortcut(QKeySequence(Qt::Key_Comma));
            p.actions["Audio/Mute"] = new QAction(this);
            p.actions["Audio/Mute"]->setCheckable(true);
            p.actions["Audio/Mute"]->setText(tr("Mute"));
            p.actions["Audio/Mute"]->setShortcut(QKeySequence(Qt::Key_M));

            auto fileMenu = new QMenu;
            fileMenu->setTitle(tr("&File"));
            fileMenu->addAction(p.actions["File/Open"]);
            fileMenu->addAction(p.actions["File/OpenWithAudio"]);
            fileMenu->addAction(p.actions["File/Close"]);
            fileMenu->addAction(p.actions["File/CloseAll"]);
            p.recentFilesMenu = new QMenu;
            p.recentFilesMenu->setTitle(tr("&Recent Files"));
            fileMenu->addMenu(p.recentFilesMenu);
            fileMenu->addSeparator();
            fileMenu->addAction(p.actions["File/Next"]);
            fileMenu->addAction(p.actions["File/Prev"]);
            fileMenu->addSeparator();
            fileMenu->addAction(p.actions["File/NextLayer"]);
            fileMenu->addAction(p.actions["File/PrevLayer"]);
            fileMenu->addSeparator();
            fileMenu->addAction(p.actions["File/Exit"]);

            auto compareMenu = new QMenu;
            compareMenu->setTitle(tr("&Compare"));
            compareMenu->addAction(p.actions["Compare/A"]);
            compareMenu->addAction(p.actions["Compare/B"]);
            compareMenu->addAction(p.actions["Compare/Wipe"]);
            compareMenu->addAction(p.actions["Compare/Tile"]);
            compareMenu->addSeparator();
            compareMenu->addAction(p.actions["Compare/Next"]);
            compareMenu->addAction(p.actions["Compare/Prev"]);

            auto windowMenu = new QMenu;
            windowMenu->setTitle(tr("&Window"));
            windowMenu->addAction(p.actions["Window/Resize1280x720"]);
            windowMenu->addAction(p.actions["Window/Resize1920x1080"]);
            windowMenu->addSeparator();
            windowMenu->addAction(p.actions["Window/FullScreen"]);
            windowMenu->addAction(p.actions["Window/FloatOnTop"]);
            windowMenu->addSeparator();
            windowMenu->addAction(p.actions["Window/Secondary"]);
            windowMenu->addAction(p.actions["Window/SecondaryFloatOnTop"]);

            auto viewMenu = new QMenu;
            viewMenu->setTitle(tr("&View"));
            viewMenu->addAction(p.actions["View/Frame"]);
            viewMenu->addAction(p.actions["View/Zoom1To1"]);
            viewMenu->addAction(p.actions["View/ZoomIn"]);
            viewMenu->addAction(p.actions["View/ZoomOut"]);

            auto imageMenu = new QMenu;
            imageMenu->setTitle(tr("&Image"));
            imageMenu->addAction(p.actions["Image/Channels/Red"]);
            imageMenu->addAction(p.actions["Image/Channels/Green"]);
            imageMenu->addAction(p.actions["Image/Channels/Blue"]);
            imageMenu->addAction(p.actions["Image/Channels/Alpha"]);
            imageMenu->addSeparator();
            imageMenu->addAction(p.actions["Image/MirrorX"]);
            imageMenu->addAction(p.actions["Image/MirrorY"]);
            imageMenu->addSeparator();
            auto yuvRangeMenu = imageMenu->addMenu(tr("YUV Range"));
            yuvRangeMenu->addAction(p.actions["Image/YUVRange/FromFile"]);
            yuvRangeMenu->addAction(p.actions["Image/YUVRange/Full"]);
            yuvRangeMenu->addAction(p.actions["Image/YUVRange/Video"]);
            auto alphaBlendMenu = imageMenu->addMenu(tr("Alpha Blend"));
            alphaBlendMenu->addAction(p.actions["Image/AlphaBlend/None"]);
            alphaBlendMenu->addAction(p.actions["Image/AlphaBlend/Straight"]);
            alphaBlendMenu->addAction(p.actions["Image/AlphaBlend/Premultiplied"]);

            auto playbackMenu = new QMenu;
            playbackMenu->setTitle(tr("&Playback"));
            playbackMenu->addAction(p.actions["Playback/Stop"]);
            playbackMenu->addAction(p.actions["Playback/Forward"]);
            playbackMenu->addAction(p.actions["Playback/Reverse"]);
            playbackMenu->addAction(p.actions["Playback/Toggle"]);
            playbackMenu->addSeparator();
            playbackMenu->addAction(p.actions["Playback/Loop"]);
            playbackMenu->addAction(p.actions["Playback/Once"]);
            playbackMenu->addAction(p.actions["Playback/PingPong"]);
            playbackMenu->addSeparator();
            playbackMenu->addAction(p.actions["Playback/Start"]);
            playbackMenu->addAction(p.actions["Playback/End"]);
            playbackMenu->addSeparator();
            playbackMenu->addAction(p.actions["Playback/FramePrev"]);
            playbackMenu->addAction(p.actions["Playback/FramePrevX10"]);
            playbackMenu->addAction(p.actions["Playback/FramePrevX100"]);
            playbackMenu->addAction(p.actions["Playback/FrameNext"]);
            playbackMenu->addAction(p.actions["Playback/FrameNextX10"]);
            playbackMenu->addAction(p.actions["Playback/FrameNextX100"]);
            playbackMenu->addSeparator();
            playbackMenu->addAction(p.actions["Playback/SetInPoint"]);
            playbackMenu->addAction(p.actions["Playback/ResetInPoint"]);
            playbackMenu->addAction(p.actions["Playback/SetOutPoint"]);
            playbackMenu->addAction(p.actions["Playback/ResetOutPoint"]);
            playbackMenu->addSeparator();
            playbackMenu->addAction(p.actions["Playback/FocusCurrentFrame"]);

            auto audioMenu = new QMenu;
            audioMenu->setTitle(tr("&Audio"));
            audioMenu->addAction(p.actions["Audio/IncreaseVolume"]);
            audioMenu->addAction(p.actions["Audio/DecreaseVolume"]);
            audioMenu->addAction(p.actions["Audio/Mute"]);

            auto toolsMenu = new QMenu;
            toolsMenu->setTitle(tr("&Tools"));

            auto menuBar = new QMenuBar;
            menuBar->addMenu(fileMenu);
            menuBar->addMenu(compareMenu);
            menuBar->addMenu(windowMenu);
            menuBar->addMenu(viewMenu);
            menuBar->addMenu(imageMenu);
            menuBar->addMenu(playbackMenu);
            menuBar->addMenu(audioMenu);
            menuBar->addMenu(toolsMenu);
            setMenuBar(menuBar);

            p.filesComboBox = new QComboBox;
            p.filesComboBox->setMinimumContentsLength(10);
            p.filesComboBox->setToolTip(tr("Set the current file"));

            auto fileToolBar = new QToolBar;
            fileToolBar->setObjectName("FileToolBar");
            fileToolBar->setIconSize(QSize(20, 20));
            fileToolBar->addWidget(p.filesComboBox);
            fileToolBar->addAction(p.actions["File/Open"]);
            fileToolBar->addAction(p.actions["File/Close"]);
            addToolBar(fileToolBar);

            p.filesBComboBox = new QComboBox;
            p.filesBComboBox->setMinimumContentsLength(10);
            p.filesBComboBox->setToolTip(tr("Set the B file"));

            auto compareToolBar = new QToolBar;
            compareToolBar->setObjectName("FileToolBar");
            compareToolBar->setIconSize(QSize(20, 20));
            compareToolBar->addWidget(p.filesBComboBox);
            compareToolBar->addAction(p.actions["Compare/A"]);
            compareToolBar->addAction(p.actions["Compare/B"]);
            compareToolBar->addAction(p.actions["Compare/Wipe"]);
            compareToolBar->addAction(p.actions["Compare/Tile"]);
            addToolBar(compareToolBar);

            p.timelineWidget = new qwidget::TimelineWidget(app->getContext());
            p.timelineWidget->slider()->setThumbnails(app->settingsObject()->hasTimelineThumbnails());
            p.timelineWidget->setTimeObject(app->timeObject());
            setCentralWidget(p.timelineWidget);

            p.filesTool = new FilesTool(p.actions, app);
            auto fileDockWidget = new QDockWidget;
            fileDockWidget->setObjectName("FilesTool");
            fileDockWidget->setWindowTitle(tr("Files"));
            fileDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            fileDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            fileDockWidget->setWidget(p.filesTool);
            fileDockWidget->hide();
            fileDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F1));
            toolsMenu->addAction(fileDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, fileDockWidget);

            p.compareTool = new CompareTool(p.actions, app);
            auto compareDockWidget = new QDockWidget;
            compareDockWidget->setObjectName("CompareTool");
            compareDockWidget->setWindowTitle(tr("Compare"));
            compareDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            compareDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            compareDockWidget->setWidget(p.compareTool);
            compareDockWidget->hide();
            compareDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F2));
            toolsMenu->addAction(compareDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, compareDockWidget);

            p.colorTool = new ColorTool(app->colorModel());
            auto colorDockWidget = new QDockWidget;
            colorDockWidget->setObjectName("ColorTool");
            colorDockWidget->setWindowTitle(tr("Color"));
            colorDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            colorDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            colorDockWidget->setWidget(p.colorTool);
            colorDockWidget->hide();
            colorDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F3));
            toolsMenu->addAction(colorDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, colorDockWidget);

            p.infoTool = new InfoTool();
            auto infoDockWidget = new QDockWidget;
            infoDockWidget->setObjectName("InfoTool");
            infoDockWidget->setWindowTitle(tr("Information"));
            infoDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            infoDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            infoDockWidget->setWidget(p.infoTool);
            infoDockWidget->hide();
            infoDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F4));
            toolsMenu->addAction(infoDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, infoDockWidget);

            p.audioTool = new AudioTool();
            auto audioDockWidget = new QDockWidget;
            audioDockWidget->setObjectName("AudioTool");
            audioDockWidget->setWindowTitle(tr("Audio"));
            audioDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            audioDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            audioDockWidget->setWidget(p.audioTool);
            audioDockWidget->hide();
            audioDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F5));
            toolsMenu->addAction(audioDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, audioDockWidget);

            p.settingsTool = new SettingsTool(app->settingsObject(), app->timeObject());
            auto settingsDockWidget = new QDockWidget;
            settingsDockWidget->setObjectName("SettingsTool");
            settingsDockWidget->setWindowTitle(tr("Settings"));
            settingsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            settingsDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            settingsDockWidget->setWidget(p.settingsTool);
            settingsDockWidget->hide();
            settingsDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F9));
            toolsMenu->addAction(settingsDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

            p.messagesTool = new MessagesTool(app->getContext());
            auto messagesDockWidget = new QDockWidget;
            messagesDockWidget->setObjectName("MessagesTool");
            messagesDockWidget->setWindowTitle(tr("Messages"));
            messagesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            messagesDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            messagesDockWidget->setWidget(p.messagesTool);
            messagesDockWidget->hide();
            messagesDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F10));
            toolsMenu->addAction(messagesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, messagesDockWidget);

            p.systemLogTool = new SystemLogTool(app->getContext());
            auto systemLogDockWidget = new QDockWidget;
            systemLogDockWidget->setObjectName("SystemLogTool");
            systemLogDockWidget->setWindowTitle(tr("System Log"));
            systemLogDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            systemLogDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            systemLogDockWidget->setWidget(p.systemLogTool);
            systemLogDockWidget->hide();
            systemLogDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F11));
            toolsMenu->addAction(systemLogDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, systemLogDockWidget);

            p.infoLabel = new QLabel;
    
            p.statusBar = new QStatusBar;
            p.statusBar->addPermanentWidget(p.infoLabel);
            setStatusBar(p.statusBar);

            _recentFilesUpdate();
            _widgetUpdate();

            p.filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >&)
                {
                    _widgetUpdate();
                });
            p.aIndexObserver = observer::ValueObserver<int>::create(
                app->filesModel()->observeAIndex(),
                [this](int)
                {
                    _widgetUpdate();
                });
            p.bIndexesObserver = observer::ListObserver<int>::create(
                app->filesModel()->observeBIndexes(),
                [this](const std::vector<int>&)
                {
                    _widgetUpdate();
                });
            p.compareOptionsObserver = observer::ValueObserver<render::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const render::CompareOptions& value)
                {
                    _p->compareOptions = value;
                    _widgetUpdate();
                });

            p.colorConfigObserver = observer::ValueObserver<imaging::ColorConfig>::create(
                app->colorModel()->observeConfig(),
                [this](const imaging::ColorConfig& value)
                {
                    _p->colorConfig = value;
                    _widgetUpdate();
                });

            p.logObserver = observer::ValueObserver<core::LogItem>::create(
                app->getContext()->getLogSystem()->observeLog(),
                [this](const core::LogItem& value)
                {
                    switch (value.type)
                    {
                    case core::LogType::Error:
                        _p->statusBar->showMessage(
                            QString(tr("ERROR: %1")).
                            arg(QString::fromUtf8(value.message.c_str())),
                            errorTimeout);
                        break;
                    default: break;
                    }
                });

            connect(
                app,
                &App::imageOptionsChanged,
                [this](const render::ImageOptions& value)
                {
                    _p->imageOptions = value;
                    _widgetUpdate();
                });

            connect(
                p.actions["File/Open"],
                &QAction::triggered,
                app,
                &App::openDialog);
            connect(
                p.actions["File/OpenWithAudio"],
                &QAction::triggered,
                app,
                &App::openWithAudioDialog);
            connect(
                p.actions["File/Close"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->close();
                });
            connect(
                p.actions["File/CloseAll"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->closeAll();
                });
            connect(
                p.recentFilesActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_recentFilesCallback(QAction*)));
            connect(
                p.actions["File/Next"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->next();
                });
            connect(
                p.actions["File/Prev"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prev();
                });
            connect(
                p.actions["File/NextLayer"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->nextLayer();
                });
            connect(
                p.actions["File/PrevLayer"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prevLayer();
                });
            connect(
                p.actions["File/Exit"],
                &QAction::triggered,
                app,
                &App::quit);

            connect(
                p.compareActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_compareCallback(QAction*)));
            connect(
                p.actions["Compare/Next"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->nextB();
                });
            connect(
                p.actions["Compare/Prev"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prevB();
                });

            connect(
                p.actions["Window/Resize1280x720"],
                &QAction::triggered,
                [this]
                {
                    resize(1280, 720);
                });
            connect(
                p.actions["Window/Resize1920x1080"],
                &QAction::triggered,
                [this]
                {
                    resize(1920, 1080);
                });
            connect(
                p.actions["Window/FullScreen"],
                &QAction::triggered,
                [this]
                {
                    setWindowState(windowState() ^ Qt::WindowFullScreen);
                });
            connect(
                p.actions["Window/FloatOnTop"],
                &QAction::toggled,
                [this](bool value)
                {
                    _p->floatOnTop = value;
                    if (_p->floatOnTop)
                    {
                        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
                    }
                    else
                    {
                        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
                    }
                    show();
                });
            connect(
                p.actions["Window/Secondary"],
                SIGNAL(toggled(bool)),
                SLOT(_secondaryWindowCallback(bool)));
            connect(
                p.actions["Window/SecondaryFloatOnTop"],
                &QAction::toggled,
                [this](bool value)
                {
                    _p->secondaryFloatOnTop = value;
                    if (_p->secondaryWindow)
                    {
                        if (_p->secondaryFloatOnTop)
                        {
                            _p->secondaryWindow->setWindowFlags(_p->secondaryWindow->windowFlags() | Qt::WindowStaysOnTopHint);
                        }
                        else
                        {
                            _p->secondaryWindow->setWindowFlags(_p->secondaryWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
                        }
                        _p->secondaryWindow->show();
                    }
                });

            connect(
                p.actions["View/Frame"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineWidget->viewport()->frameView();
                });
            connect(
                p.actions["View/Zoom1To1"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineWidget->viewport()->viewZoom1To1();
                });
            connect(
                p.actions["View/ZoomIn"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineWidget->viewport()->viewZoomIn();
                });
            connect(
                p.actions["View/ZoomOut"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineWidget->viewport()->viewZoomOut();
                });

            connect(
                p.yuvRangeActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_yuvRangeCallback(QAction*)));
            connect(
                p.channelsActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_channelsCallback(QAction*)));
            connect(
                p.alphaBlendActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_alphaBlendCallback(QAction*)));

            connect(
                p.actions["Image/MirrorX"],
                &QAction::toggled,
                [this](bool value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.mirror.x = value;
                    _p->app->setImageOptions(imageOptions);
                });
            connect(
                p.actions["Image/MirrorY"],
                &QAction::toggled,
                [this](bool value)
                {
                    render::ImageOptions imageOptions = _p->imageOptions;
                    imageOptions.mirror.y = value;
                    _p->app->setImageOptions(imageOptions);
                });

            connect(
                p.actions["Playback/Toggle"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->togglePlayback();
                    }
                });
            connect(
                p.actions["Playback/Start"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->start();
                    }
                });
            connect(
                p.actions["Playback/End"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->end();
                    }
                });
            connect(
                p.actions["Playback/FramePrev"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->framePrev();
                    }
                });
            connect(
                p.actions["Playback/FramePrevX10"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FramePrevX10);
                    }
                });
            connect(
                p.actions["Playback/FramePrevX100"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FramePrevX100);
                    }
                });
            connect(
                p.actions["Playback/FrameNext"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->frameNext();
                    }
                });
            connect(
                p.actions["Playback/FrameNextX10"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FrameNextX10);
                    }
                });
            connect(
                p.actions["Playback/FrameNextX100"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FrameNextX100);
                    }
                });
            connect(
                p.actions["Playback/FocusCurrentFrame"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineWidget->focusCurrentFrame();
                });

            connect(
                p.playbackActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_playbackCallback(QAction*)));

            connect(
                p.loopActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_loopCallback(QAction*)));

            connect(
                p.filesComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [app](int value)
                {
                    app->filesModel()->setA(value);
                });

            connect(
                p.filesBComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [app](int value)
                {
                    app->filesModel()->clearB();
                    app->filesModel()->setB(value, true);
                });

            connect(
                p.compareTool,
                &CompareTool::compareOptionsChanged,
                [app](const render::CompareOptions& value)
                {
                    app->filesModel()->setCompareOptions(value);
                });

            connect(
                p.colorTool,
                &ColorTool::imageOptionsChanged,
                [app](const render::ImageOptions& value)
                {
                    app->setImageOptions(value);
                });

            connect(
                p.audioTool,
                &AudioTool::audioOffsetChanged,
                [this](double value)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->setAudioOffset(value);
                    }
                });

            connect(
                app->settingsObject(),
                SIGNAL(recentFilesChanged(const QList<QString>&)),
                SLOT(_recentFilesCallback()));
            connect(
                app->settingsObject(),
                &SettingsObject::timelineThumbnailsChanged,
                [this](bool value)
                {
                    _p->timelineWidget->slider()->setThumbnails(value);
                });

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("MainWindow/geometry")).toByteArray();
            if (!ba.isEmpty())
            {
                restoreGeometry(settings.value(qt::versionedSettingsKey("MainWindow/geometry")).toByteArray());
            }
            else
            {
                resize(1280, 720);
            }
            ba = settings.value(qt::versionedSettingsKey("MainWindow/windowState")).toByteArray();
            if (!ba.isEmpty())
            {
                restoreState(settings.value(qt::versionedSettingsKey("MainWindow/windowState")).toByteArray());
            }
            if (settings.contains("MainWindow/FloatOnTop"))
            {
                p.floatOnTop = settings.value("MainWindow/FloatOnTop").toBool();
                if (p.floatOnTop)
                {
                    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
                }
                else
                {
                    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
                }
                QSignalBlocker blocker(p.actions["Window/FloatOnTop"]);
                p.actions["Window/FloatOnTop"]->setChecked(p.floatOnTop);
            }
            if (settings.contains("MainWindow/SecondaryFloatOnTop"))
            {
                p.secondaryFloatOnTop = settings.value("MainWindow/SecondaryFloatOnTop").toBool();
                QSignalBlocker blocker(p.actions["Window/SecondaryFloatOnTop"]);
                p.actions["Window/SecondaryFloatOnTop"]->setChecked(p.secondaryFloatOnTop);
            }
        }

        MainWindow::~MainWindow()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("MainWindow/geometry"), saveGeometry());
            settings.setValue(qt::versionedSettingsKey("MainWindow/windowState"), saveState());
            settings.setValue("MainWindow/FloatOnTop", p.floatOnTop);
            settings.setValue("MainWindow/SecondaryFloatOnTop", p.secondaryFloatOnTop);
            if (p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
            }
        }

        void MainWindow::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(loopChanged(tl::timeline::Loop)),
                    this,
                    SLOT(_loopCallback(tl::timeline::Loop)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    p.audioTool,
                    SLOT(setAudioOffset(double)));

                disconnect(
                    p.actions["Playback/SetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setInPoint()));
                disconnect(
                    p.actions["Playback/ResetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetInPoint()));
                disconnect(
                    p.actions["Playback/SetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setOutPoint()));
                disconnect(
                    p.actions["Playback/ResetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetOutPoint()));

                disconnect(
                    p.actions["Audio/IncreaseVolume"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(increaseVolume()));
                disconnect(
                    p.actions["Audio/DecreaseVolume"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(decreaseVolume()));
                disconnect(
                    p.actions["Audio/Mute"],
                    SIGNAL(toggled(bool)),
                    p.timelinePlayers[0],
                    SLOT(setMute(bool)));
            }

            p.timelinePlayers = timelinePlayers;

            if (!p.timelinePlayers.empty())
            {
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(loopChanged(tl::timeline::Loop)),
                    SLOT(_loopCallback(tl::timeline::Loop)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    p.audioTool,
                    SLOT(setAudioOffset(double)));

                connect(
                    p.actions["Playback/SetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setInPoint()));
                connect(
                    p.actions["Playback/ResetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetInPoint()));
                connect(
                    p.actions["Playback/SetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setOutPoint()));
                connect(
                    p.actions["Playback/ResetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetOutPoint()));

                connect(
                    p.actions["Audio/IncreaseVolume"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(increaseVolume()));
                connect(
                    p.actions["Audio/DecreaseVolume"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(decreaseVolume()));
                connect(
                    p.actions["Audio/Mute"],
                    SIGNAL(toggled(bool)),
                    p.timelinePlayers[0],
                    SLOT(setMute(bool)));
            }

            _widgetUpdate();
        }

        void MainWindow::closeEvent(QCloseEvent*)
        {
            TLRENDER_P();
            if (p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
            }
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
            TLRENDER_P();
            const QMimeData* mimeData = event->mimeData();
            if (mimeData->hasUrls())
            {
                const auto urlList = mimeData->urls();
                for (int i = 0; i < urlList.size(); ++i)
                {
                    const QString fileName = urlList[i].toLocalFile();
                    p.app->open(fileName);
                }
            }
        }

        void MainWindow::_recentFilesCallback(QAction* action)
        {
            TLRENDER_P();
            const auto i = p.actionToRecentFile.find(action);
            if (i != p.actionToRecentFile.end())
            {
                p.app->open(i.value());
            }
        }

        void MainWindow::_recentFilesCallback()
        {
            _recentFilesUpdate();
        }

        void MainWindow::_compareCallback(QAction* action)
        {
            TLRENDER_P();
            auto compareOptions = p.compareOptions;
            compareOptions.mode = action->data().value<render::CompareMode>();
            p.app->filesModel()->setCompareOptions(compareOptions);
        }

        void MainWindow::_secondaryWindowCallback(bool value)
        {
            TLRENDER_P();
            if (value && !p.secondaryWindow)
            {
                p.secondaryWindow = new SecondaryWindow(p.app->getContext());
                p.secondaryWindow->viewport()->setColorConfig(p.colorConfig);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);

                connect(
                    p.timelineWidget->viewport(),
                    SIGNAL(viewPosAndZoomChanged(const math::Vector2i&, float)),
                    p.secondaryWindow->viewport(),
                    SLOT(setViewPosAndZoom(const math::Vector2i&, float)));
                connect(
                    p.timelineWidget->viewport(),
                    SIGNAL(frameViewActivated()),
                    p.secondaryWindow->viewport(),
                    SLOT(frameView()));

                connect(
                    p.secondaryWindow,
                    SIGNAL(destroyed(QObject*)),
                    SLOT(_secondaryWindowDestroyedCallback()));

                if (p.secondaryFloatOnTop)
                {
                    p.secondaryWindow->setWindowFlags(p.secondaryWindow->windowFlags() | Qt::WindowStaysOnTopHint);
                }
                else
                {
                    p.secondaryWindow->setWindowFlags(p.secondaryWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
                }
                p.secondaryWindow->show();
            }
            else if (!value && p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
            }
        }

        void MainWindow::_secondaryWindowDestroyedCallback()
        {
            TLRENDER_P();
            p.secondaryWindow = nullptr;
            p.actions["Window/Secondary"]->setChecked(false);
        }

        void MainWindow::_yuvRangeCallback(QAction* action)
        {
            TLRENDER_P();
            auto imageOptions = p.imageOptions;
            imageOptions.yuvRange = action->data().value<render::YUVRange>();
            p.app->setImageOptions(imageOptions);
        }

        void MainWindow::_channelsCallback(QAction* action)
        {
            TLRENDER_P();
            auto imageOptions = p.imageOptions;
            imageOptions.channels = action->data().value<render::Channels>() != imageOptions.channels ?
                action->data().value<render::Channels>() :
                render::Channels::Color;
            p.app->setImageOptions(imageOptions);
        }

        void MainWindow::_alphaBlendCallback(QAction* action)
        {
            TLRENDER_P();
            auto imageOptions = p.imageOptions;
            imageOptions.alphaBlend = action->data().value<render::AlphaBlend>();
            p.app->setImageOptions(imageOptions);
        }

        void MainWindow::_playbackCallback(QAction* action)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                p.timelinePlayers[0]->setPlayback(action->data().value<timeline::Playback>());
            }
        }

        void MainWindow::_playbackCallback(tl::timeline::Playback value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.playbackActionGroup);
            for (auto action : p.playbackActionGroup->actions())
            {
                if (action->data().value<timeline::Playback>() == value)
                {
                    action->setChecked(true);
                    break;
                }
            }
        }

        void MainWindow::_loopCallback(QAction* action)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                p.timelinePlayers[0]->setLoop(action->data().value<timeline::Loop>());
            }
        }

        void MainWindow::_loopCallback(tl::timeline::Loop value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.loopActionGroup);
            for (auto action : p.loopActionGroup->actions())
            {
                if (action->data().value<timeline::Loop>() == value)
                {
                    action->setChecked(true);
                    break;
                }
            }
        }

        void MainWindow::_recentFilesUpdate()
        {
            TLRENDER_P();
            for (const auto& i : p.actionToRecentFile.keys())
            {
                p.recentFilesActionGroup->removeAction(i);
                i->setParent(nullptr);
                delete i;
            }
            p.actionToRecentFile.clear();
            p.recentFilesMenu->clear();
            const auto& recentFiles = p.app->settingsObject()->recentFiles();
            for (size_t i = 0; i < recentFiles.size(); ++i)
            {
                auto action = new QAction;
                const auto& file = recentFiles[i];
                action->setText(QString("%1 %2").arg(i + 1).arg(file));
                p.recentFilesActionGroup->addAction(action);
                p.actionToRecentFile[action] = file;
                p.recentFilesMenu->addAction(action);
            }
        }

        void MainWindow::_widgetUpdate()
        {
            TLRENDER_P();
            const int count = p.app->filesModel()->observeFiles()->getSize();
            p.actions["File/Close"]->setEnabled(count > 0);
            p.actions["File/CloseAll"]->setEnabled(count > 0);
            p.actions["File/Next"]->setEnabled(count > 1);
            p.actions["File/Prev"]->setEnabled(count > 1);
            p.actions["File/NextLayer"]->setEnabled(count > 0);
            p.actions["File/PrevLayer"]->setEnabled(count > 0);

            p.actions["Compare/A"]->setEnabled(count > 0);
            p.actions["Compare/B"]->setEnabled(count > 0);
            p.actions["Compare/Wipe"]->setEnabled(count > 0);
            p.actions["Compare/Tile"]->setEnabled(count > 0);
            p.actions["Compare/Next"]->setEnabled(count > 0);
            p.actions["Compare/Prev"]->setEnabled(count > 0);

            p.actions["Image/YUVRange/FromFile"]->setEnabled(count > 0);
            p.actions["Image/YUVRange/Full"]->setEnabled(count > 0);
            p.actions["Image/YUVRange/Video"]->setEnabled(count > 0);
            p.actions["Image/Channels/Red"]->setEnabled(count > 0);
            p.actions["Image/Channels/Green"]->setEnabled(count > 0);
            p.actions["Image/Channels/Blue"]->setEnabled(count > 0);
            p.actions["Image/Channels/Alpha"]->setEnabled(count > 0);
            p.actions["Image/AlphaBlend/None"]->setEnabled(count > 0);
            p.actions["Image/AlphaBlend/Straight"]->setEnabled(count > 0);
            p.actions["Image/AlphaBlend/Premultiplied"]->setEnabled(count > 0);
            p.actions["Image/MirrorX"]->setEnabled(count > 0);
            p.actions["Image/MirrorY"]->setEnabled(count > 0);

            p.actions["Playback/Stop"]->setEnabled(count > 0);
            p.actions["Playback/Forward"]->setEnabled(count > 0);
            p.actions["Playback/Reverse"]->setEnabled(count > 0);
            p.actions["Playback/Toggle"]->setEnabled(count > 0);
            p.actions["Playback/Loop"]->setEnabled(count > 0);
            p.actions["Playback/Once"]->setEnabled(count > 0);
            p.actions["Playback/PingPong"]->setEnabled(count > 0);
            p.actions["Playback/Start"]->setEnabled(count > 0);
            p.actions["Playback/End"]->setEnabled(count > 0);
            p.actions["Playback/FramePrev"]->setEnabled(count > 0);
            p.actions["Playback/FramePrevX10"]->setEnabled(count > 0);
            p.actions["Playback/FramePrevX100"]->setEnabled(count > 0);
            p.actions["Playback/FrameNext"]->setEnabled(count > 0);
            p.actions["Playback/FrameNextX10"]->setEnabled(count > 0);
            p.actions["Playback/FrameNextX100"]->setEnabled(count > 0);
            p.actions["Playback/SetInPoint"]->setEnabled(count > 0);
            p.actions["Playback/ResetInPoint"]->setEnabled(count > 0);
            p.actions["Playback/SetOutPoint"]->setEnabled(count > 0);
            p.actions["Playback/ResetOutPoint"]->setEnabled(count > 0);
            p.actions["Playback/FocusCurrentFrame"]->setEnabled(count > 0);

            p.actions["Audio/IncreaseVolume"]->setEnabled(count > 0);
            p.actions["Audio/DecreaseVolume"]->setEnabled(count > 0);
            p.actions["Audio/Mute"]->setEnabled(count > 0);

            std::vector<std::string> info;

            if (!p.timelinePlayers.empty())
            {
                {
                    QSignalBlocker blocker(p.compareActionGroup);
                    for (auto action : p.compareActionGroup->actions())
                    {
                        if (action->data().value<render::CompareMode>() == p.compareOptions.mode)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.yuvRangeActionGroup);
                    for (auto action : p.yuvRangeActionGroup->actions())
                    {
                        if (action->data().value<render::YUVRange>() == p.imageOptions.yuvRange)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.channelsActionGroup);
                    p.actions["Image/Channels/Red"]->setChecked(false);
                    p.actions["Image/Channels/Green"]->setChecked(false);
                    p.actions["Image/Channels/Blue"]->setChecked(false);
                    p.actions["Image/Channels/Alpha"]->setChecked(false);
                    for (auto action : p.channelsActionGroup->actions())
                    {
                        if (action->data().value<render::Channels>() == p.imageOptions.channels)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.alphaBlendActionGroup);
                    for (auto action : p.alphaBlendActionGroup->actions())
                    {
                        if (action->data().value<render::AlphaBlend>() == p.imageOptions.alphaBlend)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.actions["Image/MirrorX"]);
                    p.actions["Image/MirrorX"]->setChecked(p.imageOptions.mirror.x);
                }
                {
                    QSignalBlocker blocker(p.actions["Image/MirrorY"]);
                    p.actions["Image/MirrorY"]->setChecked(p.imageOptions.mirror.y);
                }
                {
                    QSignalBlocker blocker(p.playbackActionGroup);
                    for (auto action : p.playbackActionGroup->actions())
                    {
                        if (action->data().value<timeline::Playback>() == p.timelinePlayers[0]->playback())
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.loopActionGroup);
                    for (auto action : p.loopActionGroup->actions())
                    {
                        if (action->data().value<timeline::Loop>() == p.timelinePlayers[0]->loop())
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.actions["Audio/Mute"]);
                    p.actions["Audio/Mute"]->setChecked(p.timelinePlayers[0]->isMuted());
                }

                {
                    QSignalBlocker blocker(p.filesComboBox);
                    p.filesComboBox->clear();
                    for (const auto& i : p.app->filesModel()->observeFiles()->get())
                    {
                        p.filesComboBox->addItem(QString::fromUtf8(i->path.get(-1, false).c_str()));
                    }
                    p.filesComboBox->setCurrentIndex(p.app->filesModel()->observeAIndex()->get());
                }

                {
                    QSignalBlocker blocker(p.filesBComboBox);
                    p.filesBComboBox->clear();
                    for (const auto& i : p.app->filesModel()->observeFiles()->get())
                    {
                        p.filesBComboBox->addItem(QString::fromUtf8(i->path.get(-1, false).c_str()));
                    }
                    int index = 0;
                    const auto& indexes = p.app->filesModel()->observeBIndexes()->get();
                    if (!indexes.empty())
                    {
                        index = indexes[0];
                    }
                    p.filesBComboBox->setCurrentIndex(index);
                }

                const auto& avInfo = p.timelinePlayers[0]->avInfo();
                if (!avInfo.video.empty())
                {
                    std::stringstream ss;
                    ss << "Video: " << avInfo.video[0];
                    info.push_back(ss.str());
                }
                if (avInfo.audio.isValid())
                {
                    std::stringstream ss;
                    ss << "Audio: " << avInfo.audio;
                    info.push_back(ss.str());
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.compareActionGroup);
                    p.actions["Compare/A"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.yuvRangeActionGroup);
                    p.actions["Image/YUVRange/FromFile"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.channelsActionGroup);
                    p.actions["Image/Channels/Red"]->setChecked(false);
                    p.actions["Image/Channels/Green"]->setChecked(false);
                    p.actions["Image/Channels/Blue"]->setChecked(false);
                    p.actions["Image/Channels/Alpha"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(p.alphaBlendActionGroup);
                    p.actions["Image/AlphaBlend/None"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.actions["Image/MirrorX"]);
                    p.actions["Image/MirrorX"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(p.actions["Image/MirrorY"]);
                    p.actions["Image/MirrorY"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(p.playbackActionGroup);
                    p.actions["Playback/Stop"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.loopActionGroup);
                    p.actions["Playback/Loop"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.actions["Audio/Mute"]);
                    p.actions["Audio/Mute"]->setChecked(false);
                }

                {
                    QSignalBlocker blocker(p.filesComboBox);
                    p.filesComboBox->clear();
                }
                {
                    QSignalBlocker blocker(p.filesBComboBox);
                    p.filesBComboBox->clear();
                }
            }

            p.timelineWidget->setTimelinePlayers(p.timelinePlayers);
            p.timelineWidget->setColorConfig(p.colorConfig);
            std::vector<render::ImageOptions> imageOptions;
            for (const auto& i : p.timelinePlayers)
            {
                imageOptions.push_back(p.imageOptions);
            }
            p.timelineWidget->setImageOptions(imageOptions);
            p.timelineWidget->setCompareOptions(p.compareOptions);

            p.compareTool->setCompareOptions(p.compareOptions);

            p.colorTool->setImageOptions(p.imageOptions);

            p.infoTool->setInfo(!p.timelinePlayers.empty() ? p.timelinePlayers[0]->avInfo() : avio::Info());

            p.audioTool->setAudioOffset(!p.timelinePlayers.empty() ? p.timelinePlayers[0]->audioOffset() : 0.0);

            p.infoLabel->setText(QString::fromUtf8(string::join(info, " ").c_str()));

            if (p.secondaryWindow)
            {
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);
                p.secondaryWindow->viewport()->setColorConfig(p.colorConfig);
                p.secondaryWindow->viewport()->setImageOptions(imageOptions);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
            }
        }
    }
}
