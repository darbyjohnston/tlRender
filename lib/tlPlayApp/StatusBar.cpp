// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/StatusBar.h>

#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/core/Context.h>
#include <dtk/core/Timer.h>

namespace tl
{
    namespace play_app
    {
        struct StatusBar::Private
        {
            std::shared_ptr<dtk::Label> label;
            std::shared_ptr<dtk::HorizontalLayout> layout;
            std::shared_ptr<dtk::Timer> timer;
            std::function<void(void)> clickedCallback;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        void StatusBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::StatusBar",
                parent);
            DTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.label = dtk::Label::create(context);
            p.label->setMarginRole(dtk::SizeRole::MarginInside);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.label->setParent(p.layout);

            p.timer = dtk::Timer::create(context);

            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    _widgetUpdate(value);
                });
        }

        StatusBar::StatusBar() :
            _p(new Private)
        {}

        StatusBar::~StatusBar()
        {}

        std::shared_ptr<StatusBar> StatusBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StatusBar>(new StatusBar);
            out->_init(context, parent);
            return out;
        }

        void StatusBar::setClickedCallback(const std::function<void(void)>& value)
        {
            _p->clickedCallback = value;
        }

        void StatusBar::setGeometry(const dtk::Box2I & value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void StatusBar::sizeHintEvent(const dtk::SizeHintEvent & event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void StatusBar::_widgetUpdate(const std::vector<dtk::LogItem>& value)
        {
            DTK_P();
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case dtk::LogType::Error:
                {
                    const std::string s = dtk::toString(i);
                    p.label->setText(s);
                    p.label->setTooltip(s);
                    p.timer->start(
                        std::chrono::seconds(5),
                        [this]
                        {
                            _p->label->setText(std::string());
                            _p->label->setTooltip(std::string());
                        });
                    break;
                }
                default: break;
                }
            }
        }

        void StatusBar::mousePressEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            event.accept = true;
        }

        void StatusBar::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            DTK_P();
            event.accept = true;
            if (p.clickedCallback)
            {
                p.clickedCallback();
            }
        }
    }
}
