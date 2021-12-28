// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "SecondaryWindow.h"

#include <QBoxLayout>
#include <QKeyEvent>

namespace tlr
{
    SecondaryWindow::SecondaryWindow(
        const std::shared_ptr<core::Context>& context,
        QWidget* parent) :
        QWidget(parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);

        _viewport = new qwidget::TimelineViewport(context);

        auto layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->addWidget(_viewport);
        setLayout(layout);
    }

    void SecondaryWindow::setColorConfig(const imaging::ColorConfig& value)
    {
        _viewport->setColorConfig(value);
    }

    void SecondaryWindow::setTimelinePlayer(qt::TimelinePlayer* value)
    {
        _viewport->setTimelinePlayer(value);
    }

    void SecondaryWindow::keyPressEvent(QKeyEvent* event)
    {
        if (Qt::Key_Escape == event->key())
        {
            event->accept();
            close();
        }
    }
}
