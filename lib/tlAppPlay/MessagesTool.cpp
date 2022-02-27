// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/MessagesTool.h>

#include <tlAppPlay/App.h>

#include <QBoxLayout>
#include <QListWidget>
#include <QToolButton>

using namespace tl::core;

namespace tl
{
    namespace app
    {
        namespace play
        {
            namespace
            {
                const int messagesMax = 100;
            }

            struct MessagesTool::Private
            {
                QListWidget* listWidget = nullptr;
                QToolButton* clearButton = nullptr;
                std::shared_ptr<observer::ValueObserver<LogItem> > logObserver;
            };

            MessagesTool::MessagesTool(
                const std::shared_ptr<Context>& context,
                QWidget* parent) :
                ToolWidget(parent),
                _p(new Private)
            {
                TLRENDER_P();

                p.listWidget = new QListWidget;

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

                p.logObserver = observer::ValueObserver<LogItem>::create(
                    context->getLogSystem()->observeLog(),
                    [this](const LogItem& value)
                    {
                        switch (value.type)
                        {
                        case LogType::Warning:
                            _p->listWidget->addItem(QString("Warning: %1").arg(QString::fromUtf8(value.message.c_str())));
                            break;
                        case LogType::Error:
                            _p->listWidget->addItem(QString("ERROR: %1").arg(QString::fromUtf8(value.message.c_str())));
                            break;
                        default: break;
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

            MessagesTool::~MessagesTool()
            {}
        }
    }
}
