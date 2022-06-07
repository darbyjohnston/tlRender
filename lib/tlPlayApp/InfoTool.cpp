// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/InfoTool.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/DockTitleBar.h>
#include <tlPlayApp/InfoModel.h>
#include <tlPlayApp/SettingsObject.h>

#include <tlQtWidget/SearchWidget.h>

#include <tlQt/Util.h>

#include <QAction>
#include <QBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        struct InfoTool::Private
        {
            App* app = nullptr;

            VideoInfoModel* videoInfoModel = nullptr;
            QSortFilterProxyModel* videoInfoProxyModel = nullptr;
            AudioInfoModel* audioInfoModel = nullptr;
            QSortFilterProxyModel* audioInfoProxyModel = nullptr;
            TagsModel* tagsModel = nullptr;
            QSortFilterProxyModel* tagsProxyModel = nullptr;

            QTreeView* videoInfoView = nullptr;
            qtwidget::SearchWidget* videoSearchWidget = nullptr;
            QTreeView* audioInfoView = nullptr;
            qtwidget::SearchWidget* audioSearchWidget = nullptr;
            QTreeView* tagsView = nullptr;
            qtwidget::SearchWidget* tagsSearchWidget = nullptr;
        };

        InfoTool::InfoTool(
            App* app,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.videoInfoModel = new VideoInfoModel(this);
            p.videoInfoProxyModel = new QSortFilterProxyModel(this);
            p.videoInfoProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            p.videoInfoProxyModel->setFilterKeyColumn(-1);
            p.videoInfoProxyModel->setSourceModel(p.videoInfoModel);

            p.audioInfoModel = new AudioInfoModel(this);
            p.audioInfoProxyModel = new QSortFilterProxyModel(this);
            p.audioInfoProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            p.audioInfoProxyModel->setFilterKeyColumn(-1);
            p.audioInfoProxyModel->setSourceModel(p.audioInfoModel);

            p.tagsModel = new TagsModel(this);
            p.tagsProxyModel = new QSortFilterProxyModel(this);
            p.tagsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            p.tagsProxyModel->setFilterKeyColumn(-1);
            p.tagsProxyModel->setSourceModel(p.tagsModel);

            p.videoInfoView = new QTreeView;
            p.videoInfoView->setAllColumnsShowFocus(true);
            p.videoInfoView->setAlternatingRowColors(true);
            p.videoInfoView->setSelectionMode(QAbstractItemView::NoSelection);
            p.videoInfoView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.videoInfoView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.videoInfoView->setIndentation(0);
            p.videoInfoView->setModel(p.videoInfoProxyModel);

            p.videoSearchWidget = new qtwidget::SearchWidget;

            p.audioInfoView = new QTreeView;
            p.audioInfoView->setAllColumnsShowFocus(true);
            p.audioInfoView->setAlternatingRowColors(true);
            p.audioInfoView->setSelectionMode(QAbstractItemView::NoSelection);
            p.audioInfoView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.audioInfoView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.audioInfoView->setIndentation(0);
            p.audioInfoView->setModel(p.audioInfoProxyModel);

            p.audioSearchWidget = new qtwidget::SearchWidget;

            p.tagsView = new QTreeView;
            p.tagsView->setAllColumnsShowFocus(true);
            p.tagsView->setAlternatingRowColors(true);
            p.tagsView->setSelectionMode(QAbstractItemView::NoSelection);
            p.tagsView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.tagsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.tagsView->setIndentation(0);
            p.tagsView->setModel(p.tagsProxyModel);

            p.tagsSearchWidget = new qtwidget::SearchWidget;

            auto widget = new QWidget;
            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.videoInfoView);
            layout->addWidget(p.videoSearchWidget);
            widget->setLayout(layout);
            addBellows(tr("Video"), widget);

            widget = new QWidget;
            layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.audioInfoView);
            layout->addWidget(p.audioSearchWidget);
            widget->setLayout(layout);
            addBellows(tr("Audio"), widget);

            widget = new QWidget;
            layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.tagsView);
            layout->addWidget(p.tagsSearchWidget);
            widget->setLayout(layout);
            addBellows(tr("Tags"), widget);

            addStretch();

            connect(
                p.videoSearchWidget,
                SIGNAL(searchChanged(const QString&)),
                p.videoInfoProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            connect(
                p.audioSearchWidget,
                SIGNAL(searchChanged(const QString&)),
                p.audioInfoProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            connect(
                p.tagsSearchWidget,
                SIGNAL(searchChanged(const QString&)),
                p.tagsProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            app->settingsObject()->setDefaultValue("InfoTool/VideoHeader", QByteArray());
            auto ba = app->settingsObject()->value("InfoTool/VideoHeader").toByteArray();
            if (!ba.isEmpty())
            {
                p.videoInfoView->header()->restoreState(ba);
            }
            app->settingsObject()->setDefaultValue("InfoTool/AudioHeader", QByteArray());
            ba = app->settingsObject()->value("InfoTool/AudioHeader").toByteArray();
            if (!ba.isEmpty())
            {
                p.audioInfoView->header()->restoreState(ba);
            }
            app->settingsObject()->setDefaultValue("InfoTool/TagsHeader", QByteArray());
            ba = app->settingsObject()->value("InfoTool/TagsHeader").toByteArray();
            if (!ba.isEmpty())
            {
                p.tagsView->header()->restoreState(ba);
            }
        }

        InfoTool::~InfoTool()
        {
            TLRENDER_P();
            p.app->settingsObject()->setValue(
                "InfoTool/VideoHeader",
                p.videoInfoView->header()->saveState());
            p.app->settingsObject()->setValue(
                "InfoTool/AudioHeader",
                p.audioInfoView->header()->saveState());
            p.app->settingsObject()->setValue(
                "InfoTool/TagsHeader",
                p.tagsView->header()->saveState());
        }

        void InfoTool::setInfo(const io::Info& value)
        {
            TLRENDER_P();
            p.videoInfoModel->setInfo(value);
            p.audioInfoModel->setInfo(value);
            p.tagsModel->setTags(value.tags);
        }

        InfoDockWidget::InfoDockWidget(
            InfoTool* infoTool,
            QWidget* parent)
        {
            setObjectName("InfoTool");
            setWindowTitle(tr("Information"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("INFORMATION"));
            dockTitleBar->setIcon(QIcon(":/Icons/Info.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(infoTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Info.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F4));
            toggleViewAction()->setToolTip(tr("Show information"));
        }
    }
}
