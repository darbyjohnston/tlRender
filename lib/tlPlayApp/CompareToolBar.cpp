// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/CompareToolBar.h>

#include <tlPlayApp/App.h>

#include <tlPlay/FilesModel.h>

#include <dtk/ui/ButtonGroup.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct CompareToolBar::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::shared_ptr<dtk::ButtonGroup> buttonGroup;
            std::map<timeline::CompareMode, std::shared_ptr<dtk::ToolButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
        };

        void CompareToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::CompareToolBar",
                parent);
            DTK_P();

            p.actions = actions;

            p.buttonGroup = dtk::ButtonGroup::create(context, dtk::ButtonGroupType::Radio);
            const auto enums = timeline::getCompareModeEnums();
            const auto labels = timeline::getCompareModeLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto mode = enums[i];
                p.buttons[mode] = dtk::ToolButton::create(context);
                p.buttons[mode]->setCheckable(true);
                p.buttons[mode]->setIcon(p.actions[labels[i]]->icon);
                p.buttons[mode]->setTooltip(p.actions[labels[i]]->toolTip);
                p.buttonGroup->addButton(p.buttons[mode]);
            }

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            for (size_t i = 0; i < enums.size(); ++i)
            {
                p.buttons[enums[i]]->setParent(p.layout);
            }

            auto appWeak = std::weak_ptr<App>(app);
            p.buttonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (value)
                        {
                            auto options = app->getFilesModel()->getCompareOptions();
                            options.mode = static_cast<timeline::CompareMode>(index);
                            app->getFilesModel()->setCompareOptions(options);
                        }
                    }
                });

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        CompareToolBar::CompareToolBar() :
            _p(new Private)
        {}

        CompareToolBar::~CompareToolBar()
        {}

        std::shared_ptr<CompareToolBar> CompareToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareToolBar>(new CompareToolBar);
            out->_init(context, app, actions, parent);
            return out;
        }

        void CompareToolBar::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CompareToolBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void CompareToolBar::_compareUpdate(const timeline::CompareOptions& value)
        {
            DTK_P();
            p.buttonGroup->setChecked(static_cast<int>(value.mode), true);
        }
    }
}
