// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/MessagesTool.h>

#include <tlUI/IClipboard.h>
#include <tlUI/IWindow.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace play_app
    {
        namespace
        {
            const int messagesMax = 20;
        }

        struct MessagesTool::Private
        {
            std::list<std::string> messages;
            std::shared_ptr<ui::Label> label;
            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<ui::ToolButton> copyButton;
            std::shared_ptr<ui::ToolButton> clearButton;
            std::shared_ptr<ui::VerticalLayout> layout;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        void MessagesTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Messages,
                "tl::play_app::MessagesTool",
                parent);
            DTK_P();

            p.label = ui::Label::create(context);
            p.label->setFontRole(ui::FontRole::Mono);
            p.label->setMarginRole(ui::SizeRole::MarginSmall);
            p.label->setVAlign(ui::VAlign::Top);

            p.scrollWidget = ui::ScrollWidget::create(context);
            p.scrollWidget->setWidget(p.label);
            p.scrollWidget->setVStretch(ui::Stretch::Expanding);

            p.copyButton = ui::ToolButton::create("Copy", context);

            p.clearButton = ui::ToolButton::create("Clear", context);

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.scrollWidget->setParent(p.layout);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
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
                        switch (i.type)
                        {
                        case dtk::LogType::Warning:
                        case dtk::LogType::Error:
                            _p->messages.push_back(dtk::toString(i));
                            break;
                        default: break;
                        }
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

        MessagesTool::MessagesTool() :
            _p(new Private)
        {}

        MessagesTool::~MessagesTool()
        {}

        std::shared_ptr<MessagesTool> MessagesTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MessagesTool>(new MessagesTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
