// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/StatusBar.h>

#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <tlCore/Timer.h>

#include <dtk/core/Context.h>

namespace tl
{
    namespace play_app
    {
        struct StatusBar::Private
        {
            std::shared_ptr<ui::Label> label;
            std::shared_ptr<ui::HorizontalLayout> layout;
            std::shared_ptr<time::Timer> timer;
            std::function<void(void)> clickedCallback;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        void StatusBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                "tl::play_app::StatusBar",
                context,
                parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true);

            p.label = ui::Label::create(context);
            p.label->setMarginRole(ui::SizeRole::MarginInside);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.label->setParent(p.layout);

            p.timer = time::Timer::create(context);

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

        void StatusBar::setGeometry(const math::Box2i & value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void StatusBar::sizeHintEvent(const ui::SizeHintEvent & event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void StatusBar::_widgetUpdate(const std::vector<dtk::LogItem>& value)
        {
            TLRENDER_P();
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case dtk::LogType::Error:
                {
                    const std::string s = dtk::toString(i);
                    p.label->setText(s);
                    p.label->setToolTip(s);
                    p.timer->start(
                        std::chrono::seconds(5),
                        [this]
                        {
                            _p->label->setText(std::string());
                            _p->label->setToolTip(std::string());
                        });
                    break;
                }
                default: break;
                }
            }
        }

        void StatusBar::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            event.accept = true;
        }

        void StatusBar::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            event.accept = true;
            if (p.clickedCallback)
            {
                p.clickedCallback();
            }
        }
    }
}
