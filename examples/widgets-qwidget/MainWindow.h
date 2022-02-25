// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>

#include <QMainWindow>

namespace tl
{
    namespace widgets_qwidget
    {
        class MainWindow : public QMainWindow
        {
            Q_OBJECT

        public:
            MainWindow(const std::shared_ptr<core::Context>&);
        };
    }
}
