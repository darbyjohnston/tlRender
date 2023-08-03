// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/InfoTool.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct InfoTool::Private
        {
            io::Info info;
            std::string filter;

            std::shared_ptr<ui::LineEdit> filterEdit;
            std::shared_ptr<ui::ToolButton> filterClearButton;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void InfoTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Info,
                "tl::play_gl::InfoTool",
                app,
                context,
                parent);
            TLRENDER_P();

            p.filterEdit = ui::LineEdit::create(context);
            p.filterEdit->setHStretch(ui::Stretch::Expanding);
            p.filterEdit->setToolTip("Filter the information");

            p.filterClearButton = ui::ToolButton::create(context);
            p.filterClearButton->setIcon("Clear");
            p.filterClearButton->setToolTip("Clear the filter");

            p.layout = ui::GridLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(p.layout);
            scrollWidget->setVStretch(ui::Stretch::Expanding);

            auto layout = ui::VerticalLayout::create(context);
            layout->setSpacingRole(ui::SizeRole::None);
            scrollWidget->setParent(layout);
            auto hLayout = ui::HorizontalLayout::create(context, layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.filterEdit->setParent(hLayout);
            p.filterClearButton->setParent(hLayout);
            _setWidget(layout);

            p.playerObserver = observer::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->observeActivePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _p->info = (!value.empty() && value[0]) ?
                        value[0]->getIOInfo() :
                        io::Info();
                    _widgetUpdate();
                });

            p.filterEdit->setTextChangedCallback(
                [this](const std::string& value)
                {
                    _p->filter = value;
                    _widgetUpdate();
                });

            p.filterClearButton->setClickedCallback(
                [this]
                {
                    _p->filter = std::string();
                    _widgetUpdate();
                });
        }

        InfoTool::InfoTool() :
            _p(new Private)
        {}

        InfoTool::~InfoTool()
        {}

        std::shared_ptr<InfoTool> InfoTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<InfoTool>(new InfoTool);
            out->_init(app, context, parent);
            return out;
        }

        void InfoTool::_widgetUpdate()
        {
            TLRENDER_P();
            auto children = p.layout->getChildren();
            for (const auto& child : children)
            {
                child->setParent(nullptr);
            }
            if (auto context = _context.lock())
            {
                int row = 0;
                for (const auto& tag : p.info.tags)
                {
                    bool filter = false;
                    if (!p.filter.empty() &&
                        !string::contains(
                            tag.first,
                            p.filter,
                            string::Compare::CaseInsensitive) &&
                        !string::contains(
                            tag.second,
                            p.filter,
                            string::Compare::CaseInsensitive))
                    {
                        filter = true;
                    }
                    if (!filter)
                    {
                        auto label = ui::Label::create(tag.first + ":", context, p.layout);
                        p.layout->setGridPos(label, row, 0);
                        label = ui::Label::create(tag.second, context, p.layout);
                        p.layout->setGridPos(label, row, 1);
                        ++row;
                    }
                }
            }
        }
    }
}
