// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlPlay/FilesModel.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! File widget.
        class FileWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileWidget(
                const std::shared_ptr<play::FilesModelItem>&,
                QWidget* parent = nullptr);

            virtual ~FileWidget();

            void setA(bool);
            void setB(bool);
            void setLayer(int);

        Q_SIGNALS:
            void aChanged(bool);
            void bChanged(bool);
            void layerChanged(int);

        private:
            TLRENDER_PRIVATE();
        };

        //! Files tool.
        class FilesTool : public IToolWidget
        {
            Q_OBJECT

        public:
            FilesTool(
                const QMap<QString, QAction*>&,
                App*,
                QWidget* parent = nullptr);

            virtual ~FilesTool();

        private:
            void _filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<play::FilesModelItem>&);
            void _bUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);

            TLRENDER_PRIVATE();
        };

        //! Files tool dock widget.
        class FilesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            FilesDockWidget(
                FilesTool*,
                QWidget* parent = nullptr);
        };
    }
}
