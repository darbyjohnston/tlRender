// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SecondaryWindow.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/SettingsObject.h>

#include <tlQtWidget/TimelineViewport.h>

#include <QBoxLayout>
#include <QKeyEvent>

namespace tl
{
    namespace play
    {
        struct SecondaryWindow::Private
        {
            App* app = nullptr;
            qtwidget::TimelineViewport* viewport = nullptr;
        };

        SecondaryWindow::SecondaryWindow(
            App* app,
            QWidget* parent) :
            QWidget(parent),
            _p(new  Private)
        {
            TLRENDER_P();

            p.app = app;

            setAttribute(Qt::WA_DeleteOnClose);

            p.viewport = new qtwidget::TimelineViewport(app->getContext());

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.viewport);
            setLayout(layout);

            app->settingsObject()->setDefaultValue("SecondaryWindow/geometry", QByteArray());
            auto ba = app->settingsObject()->value("SecondaryWindow/geometry").toByteArray();
            if (!ba.isEmpty())
            {
                restoreGeometry(ba);
            }
            else
            {
                resize(1280, 720);
            }
        }

        SecondaryWindow::~SecondaryWindow()
        {
            _p->app->settingsObject()->setValue("SecondaryWindow/geometry", saveGeometry());
        }

        qtwidget::TimelineViewport* SecondaryWindow::viewport() const
        {
            return _p->viewport;
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
