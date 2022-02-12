// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/SecondaryWindow.h>

#include <tlQWidget/TimelineViewport.h>

#include <QBoxLayout>
#include <QKeyEvent>
#include <QSettings>

namespace tl
{
    namespace play
    {
        struct SecondaryWindow::Private
        {
            qwidget::TimelineViewport* viewport = nullptr;
        };

        SecondaryWindow::SecondaryWindow(
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            QWidget(parent),
            _p(new  Private)
        {
            TLRENDER_P();

            setAttribute(Qt::WA_DeleteOnClose);

            p.viewport = new qwidget::TimelineViewport(context);

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.viewport);
            setLayout(layout);

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("SecondaryWindow/geometry")).toByteArray();
            if (!ba.isEmpty())
            {
                restoreGeometry(settings.value(qt::versionedSettingsKey("SecondaryWindow/geometry")).toByteArray());
            }
            else
            {
                resize(1280, 720);
            }
        }

        SecondaryWindow::~SecondaryWindow()
        {
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("SecondaryWindow/geometry"), saveGeometry());
        }

        void SecondaryWindow::setColorConfig(const imaging::ColorConfig& value)
        {
            _p->viewport->setColorConfig(value);
        }

        void SecondaryWindow::setImageOptions(const std::vector<render::ImageOptions>& value)
        {
            _p->viewport->setImageOptions(value);
        }

        void SecondaryWindow::setCompareOptions(const render::CompareOptions& value)
        {
            _p->viewport->setCompareOptions(value);
        }

        void SecondaryWindow::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& value)
        {
            _p->viewport->setTimelinePlayers(value);
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
}
