// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/ToolBarButton.h>

namespace tl
{
    namespace play
    {
        struct ToolBarButton::Private
        {
            std::shared_ptr<dtk::Action> action;
            std::shared_ptr<dtk::ValueObserver<std::string> > iconObserver;
            std::shared_ptr<dtk::ValueObserver<std::string> > checkedIconObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > checkableObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > checkedObserver;
            std::shared_ptr<dtk::ValueObserver<std::string> > tooltipObserver;
        };

        void ToolBarButton::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Action>& action,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolButton::_init(context, parent);
            DTK_P();
            
            p.action = action;

            setClickedCallback(
                [this]
                {
                    _p->action->doCallback();
                });

            setCheckedCallback(
                [this](bool value)
                {
                    _p->action->doCheckedCallback(value);
                });

            p.iconObserver = dtk::ValueObserver<std::string>::create(
                action->observeIcon(),
                [this](const std::string& value)
                {
                    setIcon(value);
                });
            p.checkedIconObserver = dtk::ValueObserver<std::string>::create(
                action->observeCheckedIcon(),
                [this](const std::string& value)
                {
                    setCheckedIcon(value);
                });
            p.checkableObserver = dtk::ValueObserver<bool>::create(
                action->observeCheckable(),
                [this](bool value)
                {
                    setCheckable(value);
                });
            p.checkedObserver = dtk::ValueObserver<bool>::create(
                action->observeChecked(),
                [this](bool value)
                {
                    setChecked(value);
                });
            p.tooltipObserver = dtk::ValueObserver<std::string>::create(
                action->observeTooltip(),
                [this](const std::string& value)
                {
                    setTooltip(value);
                });
        }

        ToolBarButton::ToolBarButton() :
            _p(new Private)
        {}

        ToolBarButton::~ToolBarButton()
        {}

        std::shared_ptr<ToolBarButton> ToolBarButton::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Action>& action,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolBarButton>(new ToolBarButton);
            out->_init(context, action, parent);
            return out;
        }
    }
}
