// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SystemLogTool.h>

#include <tlPlayApp/App.h>

#include <tlQtWidget/Util.h>

#include <QBoxLayout>
#include <QListWidget>
#include <QToolButton>

namespace tl
{
    namespace play
    {
        namespace
        {
            const int messagesMax = 100;
        }

        struct SystemLogTool::Private
        {
            QListWidget* listWidget = nullptr;
            QToolButton* clearButton = nullptr;
            std::shared_ptr<observer::ValueObserver<log::Item> > logObserver;
        };

        SystemLogTool::SystemLogTool(
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.listWidget = new QListWidget;
            const QFont fixedFont = qtwidget::font("NotoMono-Regular");
            p.listWidget->setFont(fixedFont);

            p.clearButton = new QToolButton;
            p.clearButton->setIcon(QIcon(":/Icons/Clear.svg"));
            p.clearButton->setAutoRaise(true);
            p.clearButton->setToolTip(tr("Clear the messages"));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.listWidget);
            auto hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addStretch();
            hLayout->addWidget(p.clearButton);
            layout->addLayout(hLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);

            p.logObserver = observer::ValueObserver<log::Item>::create(
                context->getLogSystem()->observeLog(),
                [this](const log::Item& value)
                {
                    switch (value.type)
                    {
                    case log::Type::Message:
                        _p->listWidget->addItem(QString("%1 %2: %3").
                            arg(value.time).
                            arg(QString::fromUtf8(value.prefix.c_str())).
                            arg(QString::fromUtf8(value.message.c_str())));
                        break;
                    case log::Type::Warning:
                        _p->listWidget->addItem(QString("%1 Warning %2: %3").
                            arg(value.time).
                            arg(QString::fromUtf8(value.prefix.c_str())).
                            arg(QString::fromUtf8(value.message.c_str())));
                        break;
                    case log::Type::Error:
                        _p->listWidget->addItem(QString("%1 ERROR %2: %3").
                            arg(value.time).
                            arg(QString::fromUtf8(value.prefix.c_str())).
                            arg(QString::fromUtf8(value.message.c_str())));
                        break;
                    }
                    while (_p->listWidget->count() > messagesMax)
                    {
                        delete _p->listWidget->takeItem(0);
                    }
                });

            connect(
                p.clearButton,
                &QToolButton::clicked,
                p.listWidget,
                &QListWidget::clear);
        }

        SystemLogTool::~SystemLogTool()
        {}
    }
}
