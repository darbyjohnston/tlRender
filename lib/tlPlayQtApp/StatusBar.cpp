// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/StatusBar.h>

#include <tlPlayQtApp/App.h>

#include <tlPlay/Info.h>

#include <tlQtWidget/Divider.h>

#include <tlQt/TimelinePlayer.h>

#include <QBoxLayout>
#include <QLabel>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

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
#if defined(TLRENDER_BMD)
            QLabel* deviceActiveLabel = nullptr;
#endif // TLRENDER_BMD

            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
#if defined(TLRENDER_BMD)
            std::shared_ptr<dtk::ValueObserver<bool> > bmdActiveObserver;
#endif // TLRENDER_BMD
        };

        StatusBar::StatusBar(App* app, QWidget* parent) :
            QStatusBar(parent),
            _p(new Private)
        {
            DTK_P();

            p.infoLabel = new QLabel;

#if defined(TLRENDER_BMD)
            p.deviceActiveLabel = new QLabel;
            p.deviceActiveLabel->setPixmap(QIcon(":/Icons/Devices.svg").pixmap(QSize(20, 20)));
            p.deviceActiveLabel->setToolTip("Output device active");
#endif // TLRENDER_BMD

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(new qtwidget::Divider(Qt::Vertical));
            layout->addWidget(p.infoLabel);
#if defined(TLRENDER_BMD)
            layout->addWidget(new qtwidget::Divider(Qt::Vertical));
            layout->addWidget(p.deviceActiveLabel);
#endif // TLRENDER_BMD
            auto widget = new QWidget;
            widget->setLayout(layout);
            addPermanentWidget(widget);

            auto player = app->player();
            _infoUpdate(
                player ? player->path() : file::Path(),
                player ? player->ioInfo() : io::Info());
            _deviceUpdate(false);

            connect(
                app,
                &App::playerChanged,
                [this](const QSharedPointer<qt::TimelinePlayer>& player)
                {
                    _infoUpdate(
                        player ? player->path() : file::Path(),
                        player ? player->ioInfo() : io::Info());
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

#if defined(TLRENDER_BMD)
            p.bmdActiveObserver = dtk::ValueObserver<bool>::create(
                app->bmdOutputDevice()->observeActive(),
                [this](bool value)
                {
                    _deviceUpdate(value);
                });
#endif // TLRENDER_BMD
        }

        StatusBar::~StatusBar()
        {}

        void StatusBar::_infoUpdate(const file::Path& path, const io::Info& info)
        {
            DTK_P();
            const std::string text = play::infoLabel(path, info);
            const std::string toolTip = play::infoToolTip(path, info);
            p.infoLabel->setText(QString::fromUtf8(text.c_str()));
            p.infoLabel->setToolTip(QString::fromUtf8(toolTip.c_str()));
        }

        void StatusBar::_deviceUpdate(bool value)
        {
            DTK_P();
#if defined(TLRENDER_BMD)
            p.deviceActiveLabel->setEnabled(value);
            p.deviceActiveLabel->setAutoFillBackground(value);
            p.deviceActiveLabel->setBackgroundRole(value ? QPalette::Highlight : QPalette::NoRole);
#endif // TLRENDER_BMD
        }
    }
}
