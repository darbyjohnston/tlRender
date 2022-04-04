// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolWidget.h>

#include <tlQtWidget/BellowsWidget.h>
#include <tlQtWidget/Separator.h>

#include <QBoxLayout>
#include <QScrollArea>

namespace tl
{
    namespace play
    {
        ToolWidget::ToolWidget(QWidget* parent) :
            QWidget(parent)
        {
            _layout = new QVBoxLayout;
            _layout->setContentsMargins(0, 0, 0, 0);
            _layout->setSpacing(0);
            auto scrollWidget = new QWidget;
            scrollWidget->setLayout(_layout);
            auto scrollArea = new QScrollArea;
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(scrollWidget);
            auto scrollLayout = new QVBoxLayout;
            scrollLayout->addWidget(scrollArea);
            scrollLayout->setContentsMargins(0, 0, 0, 0);
            setLayout(scrollLayout);
        }

        void ToolWidget::addWidget(QWidget* widget, int stretch)
        {
            if (_layout->count() > 0)
            {
                _layout->addWidget(new qtwidget::Separator(Qt::Horizontal));
            }
            _layout->addWidget(widget, stretch);
        }

        void ToolWidget::addBellows(const QString& title, QWidget* widget)
        {
            if (_layout->count() > 0)
            {
                _layout->addWidget(new qtwidget::Separator(Qt::Horizontal));
            }
            auto bellowsWidget = new qtwidget::BellowsWidget;
            bellowsWidget->setTitle(title);
            bellowsWidget->setWidget(widget);
            _layout->addWidget(bellowsWidget);
        }

        void ToolWidget::addStretch(int stretch)
        {
            _layout->addStretch(stretch);
        }
    }
}
