// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/StatusBar.h>

#include <tlPlayQtApp/App.h>

#include <tlPlay/Info.h>

#include <tlQtWidget/Divider.h>

#include <QBoxLayout>
#include <QLabel>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const size_t errorTimeout = 5000;
        }

        struct StatusBar::Private
        {
            QLabel* infoLabel = nullptr;

            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        StatusBar::StatusBar(App* app, QWidget* parent) :
            QStatusBar(parent),
            _p(new Private)
        {
            DTK_P();

            p.infoLabel = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(new qtwidget::Divider(Qt::Vertical));
            layout->addWidget(p.infoLabel);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addPermanentWidget(widget);

            _playerUpdate(app->player());

            connect(
                app,
                &App::playerChanged,
                [this](const QSharedPointer<qt::TimelinePlayer>& value)
                {
                    _playerUpdate(value);
                });

            auto context = app->getContext();
            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case dtk::LogType::Error:
                            showMessage(
                                QString::fromUtf8(dtk::toString(i).c_str()),
                                errorTimeout);
                            break;
                        default: break;
                        }
                    }
                });
        }

        StatusBar::~StatusBar()
        {}

        void StatusBar::_playerUpdate(const QSharedPointer<qt::TimelinePlayer>& player)
        {
            DTK_P();
            std::string text;
            std::string toolTip;
            if (player)
            {
                const file::Path& path = player->path();
                const io::Info& info = player->ioInfo();
                text = play::infoLabel(path, info);
                toolTip = play::infoToolTip(path, info);
            }
            p.infoLabel->setText(QString::fromUtf8(text.c_str()));
            p.infoLabel->setToolTip(QString::fromUtf8(toolTip.c_str()));
        }
    }
}
