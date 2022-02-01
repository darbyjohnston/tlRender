// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/Util.h>

#include <tlrQt/Util.h>

#include <QDir>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlrQWidget);
}

namespace tlr
{
    namespace qwidget
    {
        void init()
        {
            qt::init();

            qtInitResources();
        }

        QPalette darkStyle()
        {
            QPalette palette;
            palette.setColor(QPalette::ColorRole::Window, QColor(30, 30, 30));
            palette.setColor(QPalette::ColorRole::WindowText, QColor(240, 240, 240));
            palette.setColor(QPalette::ColorRole::Base, QColor(50, 50, 50));
            palette.setColor(QPalette::ColorRole::AlternateBase, QColor(60, 60, 60));
            palette.setColor(QPalette::ColorRole::Text, QColor(240, 240, 240));
            palette.setColor(QPalette::ColorRole::Button, QColor(40, 40, 40));
            palette.setColor(QPalette::ColorRole::ButtonText, QColor(240, 240, 240));
            palette.setColor(QPalette::ColorRole::BrightText, QColor(240, 240, 240));
            palette.setColor(QPalette::ColorRole::Light, QColor(50, 50, 50));
            palette.setColor(QPalette::ColorRole::Midlight, QColor(45, 45, 45));
            palette.setColor(QPalette::ColorRole::Dark, QColor(30, 30, 30));
            palette.setColor(QPalette::ColorRole::Mid, QColor(35, 35, 35));
            palette.setColor(QPalette::ColorRole::Highlight, QColor(220, 180, 60));
            palette.setColor(QPalette::ColorRole::HighlightedText, QColor(240, 240, 240));
            return palette;
        }
    }
}

