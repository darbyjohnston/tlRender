// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/OutputTool.h>

#include <QBoxLayout>

namespace tl
{
    namespace play
    {
        struct OutputWidget::Private
        {
        };

        OutputWidget::OutputWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            auto layout = new QVBoxLayout;
            layout->addStretch();
            setLayout(layout);
        }

        OutputWidget::~OutputWidget()
        {}

        struct OutputTool::Private
        {
            OutputWidget* outputWidget = nullptr;
        };

        OutputTool::OutputTool(QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.outputWidget = new OutputWidget;

            addWidget(p.outputWidget);
            addStretch();
        }

        OutputTool::~OutputTool()
        {}
    }
}
