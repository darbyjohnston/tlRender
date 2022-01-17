// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "LayersModel.h"

#include <QListView>
#include <QToolButton>

namespace tlr
{
    //! Layers tool.
    class LayersTool : public QWidget
    {
        Q_OBJECT

    public:
        LayersTool(
            LayersModel*,
            QWidget* parent = nullptr);

    private Q_SLOTS:
        void _activatedCallback(const QModelIndex&);
        void _countCallback();

    private:
        void _countUpdate();

        LayersModel* _layersModel = nullptr;
        QListView* _listView = nullptr;
        QToolButton* _nextButton = nullptr;
        QToolButton* _prevButton = nullptr;
    };
}
