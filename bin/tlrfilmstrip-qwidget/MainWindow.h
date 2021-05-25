// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "SettingsObject.h"

#include <tlrQt/FilmstripWidget.h>

#include <tlrCore/Timeline.h>

#include <QAction>
#include <QActionGroup>
#include <QMainWindow>
#include <QScrollArea>
#include <QVBoxLayout>

namespace tlr
{
    //! Main window.
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(
            SettingsObject*,
            qt::TimeObject*,
            QWidget* parent = nullptr);

    protected:
        void closeEvent(QCloseEvent*) override;
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dragLeaveEvent(QDragLeaveEvent*) override;
        void dropEvent(QDropEvent*) override;

    private Q_SLOTS:
        void _openCallback();
        void _openedCallback(const std::shared_ptr<tlr::timeline::Timeline>&);
        void _closeAllCallback();
        void _closedCallback(const std::shared_ptr<tlr::timeline::Timeline>&);
        void _recentFilesCallback(QAction*);
        void _recentFilesCallback();
        void _saveSettingsCallback();

    private:
        void _recentFilesUpdate();

        QList<std::shared_ptr<timeline::Timeline> > _timelines;
        QMap<QString, QAction*> _actions;
        QActionGroup* _recentFilesActionGroup = nullptr;
        QMap<QAction*, QString> _actionToRecentFile;
        QMenu* _recentFilesMenu = nullptr;
        QMap<std::shared_ptr<timeline::Timeline>, qt::FilmstripWidget*> _filmstripWidgets;
        QScrollArea* _scrollArea = nullptr;
        QVBoxLayout* _scrollLayout = nullptr;
        SettingsObject* _settingsObject = nullptr;
        qt::TimeObject* _timeObject = nullptr;
    };
}
