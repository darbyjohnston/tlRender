// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/SystemLogTool.h>

#include <dtk/ui/Divider.h>
#include <dtk/ui/IClipboard.h>
#include <dtk/ui/IWindow.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>
#include <dtk/ui/ToolButton.h>
#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

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
            std::list<std::string> messages;
            std::shared_ptr<dtk::Label> label;
            std::shared_ptr<dtk::ScrollWidget> scrollWidget;
            std::shared_ptr<dtk::ToolButton> copyButton;
            std::shared_ptr<dtk::ToolButton> clearButton;
            std::shared_ptr<dtk::VerticalLayout> layout;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        void SystemLogTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::SystemLog,
                "tl::play_app::SystemLogTool",
                parent);
            DTK_P();

            p.label = dtk::Label::create(context);
            p.label->setFontRole(dtk::FontRole::Mono);
            p.label->setMarginRole(dtk::SizeRole::MarginSmall);
            p.label->setVAlign(dtk::VAlign::Top);

            p.scrollWidget = dtk::ScrollWidget::create(context);
            p.scrollWidget->setWidget(p.label);
            p.scrollWidget->setBorder(false);
            p.scrollWidget->setVStretch(dtk::Stretch::Expanding);

            p.copyButton = dtk::ToolButton::create(context, "Copy");

            p.clearButton = dtk::ToolButton::create(context, "Clear");

            p.layout = dtk::VerticalLayout::create(context);
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.scrollWidget->setParent(p.layout);
            dtk::Divider::create(context, dtk::Orientation::Vertical, p.layout);
            auto hLayout = dtk::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(dtk::SizeRole::MarginInside);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.copyButton->setParent(hLayout);
            p.clearButton->setParent(hLayout);
            _setWidget(p.layout);

            p.copyButton->setClickedCallback(
                [this]
                {
                    if (auto window = getWindow())
                    {
                        if (auto clipboard = window->getClipboard())
                        {
                            const std::string text = dtk::join(
                                { std::begin(_p->messages), std::end(_p->messages) },
                                '\n');
                            clipboard->setText(text);
                        }
                    }
                });

            p.clearButton->setClickedCallback(
                [this]
                {
                    _p->messages.clear();
                    _p->label->setText(std::string());
                });

            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    for (const auto& i : value)
                    {
                        _p->messages.push_back(dtk::toString(i));
                    }
                    while (_p->messages.size() > messagesMax)
                    {
                        _p->messages.pop_front();
                    }
                    _p->label->setText(dtk::join(
                        { std::begin(_p->messages), std::end(_p->messages) },
                        '\n'));
                });
        }

        SystemLogTool::SystemLogTool() :
            _p(new Private)
        {}

        SystemLogTool::~SystemLogTool()
        {}

        std::shared_ptr<SystemLogTool> SystemLogTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SystemLogTool>(new SystemLogTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
