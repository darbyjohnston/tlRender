// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SystemLogTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/Util.h>

#include <QAction>
#include <QBoxLayout>
#include <QClipboard>
#include <QListWidget>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const int messagesMax = 100;
        }

        struct SystemLogTool::Private
        {
            QListWidget* listWidget = nullptr;
            QToolButton* copyButton = nullptr;
            QToolButton* clearButton = nullptr;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        SystemLogTool::SystemLogTool(
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.listWidget = new QListWidget;
            p.listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            const QFont fixedFont("Noto Mono");
            p.listWidget->setFont(fixedFont);

            p.copyButton = new QToolButton;
            p.copyButton->setIcon(QIcon(":/Icons/Copy.svg"));
            p.copyButton->setAutoRaise(true);
            p.copyButton->setToolTip(tr("Copy the contents to the clipboard"));

            p.clearButton = new QToolButton;
            p.clearButton->setIcon(QIcon(":/Icons/Clear.svg"));
            p.clearButton->setAutoRaise(true);
            p.clearButton->setToolTip(tr("Clear the contents"));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.listWidget);
            auto hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addStretch();
            hLayout->addWidget(p.copyButton);
            hLayout->addWidget(p.clearButton);
            layout->addLayout(hLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);

            p.logObserver = observer::ListObserver<log::Item>::create(
                context->getLogSystem()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Message:
                            _p->listWidget->addItem(QString("%1 %2: %3").
                                arg(i.time).
                                arg(QString::fromUtf8(i.prefix.c_str())).
                                arg(QString::fromUtf8(i.message.c_str())));
                            break;
                        case log::Type::Warning:
                            _p->listWidget->addItem(QString("%1 Warning %2: %3").
                                arg(i.time).
                                arg(QString::fromUtf8(i.prefix.c_str())).
                                arg(QString::fromUtf8(i.message.c_str())));
                            break;
                        case log::Type::Error:
                            _p->listWidget->addItem(QString("%1 ERROR %2: %3").
                                arg(i.time).
                                arg(QString::fromUtf8(i.prefix.c_str())).
                                arg(QString::fromUtf8(i.message.c_str())));
                            break;
                        }
                        while (_p->listWidget->count() > messagesMax)
                        {
                            delete _p->listWidget->takeItem(0);
                        }
                    }
                });

            connect(
                p.copyButton,
                &QToolButton::clicked,
                [this]
                {
                    auto clipboard = QGuiApplication::clipboard();
                    QStringList text;
                    for (int i = 0; i < _p->listWidget->count(); ++i)
                    {
                        text.append(_p->listWidget->item(i)->text());
                    }
                    clipboard->setText(text.join('\n'));
                });

            connect(
                p.clearButton,
                &QToolButton::clicked,
                p.listWidget,
                &QListWidget::clear);
        }

        SystemLogTool::~SystemLogTool()
        {}

        SystemLogDockWidget::SystemLogDockWidget(
            SystemLogTool * systemLogTool,
            QWidget * parent)
        {
            setObjectName("SystemLogTool");
            setWindowTitle(tr("System Log"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("SYSTEM LOG"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(systemLogTool);

            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F11));
            toggleViewAction()->setToolTip(tr("Show system log"));
        }
    }
}
