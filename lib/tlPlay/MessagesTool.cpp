// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/MessagesTool.h>

#include <tlPlay/App.h>

#include <QBoxLayout>

namespace tl
{
    namespace play
    {
        namespace
        {
            const int messagesMax = 100;
        }

        MessagesTool::MessagesTool(
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            ToolWidget(parent)
        {
            _listWidget = new QListWidget;

            _clearButton = new QToolButton;
            _clearButton->setIcon(QIcon(":/Icons/Clear.svg"));
            _clearButton->setAutoRaise(true);
            _clearButton->setToolTip(tr("Clear the messages"));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(_listWidget);
            auto hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addStretch();
            hLayout->addWidget(_clearButton);
            layout->addLayout(hLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);

            _logObserver = observer::ValueObserver<core::LogItem>::create(
                context->getLogSystem()->observeLog(),
                [this](const core::LogItem& value)
                {
                    switch (value.type)
                    {
                    case core::LogType::Warning:
                        _listWidget->addItem(QString("Warning: %1").arg(QString::fromUtf8(value.message.c_str())));
                        break;
                    case core::LogType::Error:
                        _listWidget->addItem(QString("ERROR: %1").arg(QString::fromUtf8(value.message.c_str())));
                        break;
                    default: break;
                    }
                    while (_listWidget->count() > messagesMax)
                    {
                        delete _listWidget->takeItem(0);
                    }
                });

            connect(
                _clearButton,
                &QToolButton::clicked,
                _listWidget,
                &QListWidget::clear);
        }
    }
}
