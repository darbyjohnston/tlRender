// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/StatusBar.h>

#include <tlPlayApp/App.h>

#include <tlPlay/Info.h>

#include <dtk/ui/Divider.h>
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
            std::shared_ptr<dtk::Label> logLabel;
            std::shared_ptr<dtk::Label> infoLabel;
            std::shared_ptr<dtk::HorizontalLayout> layout;
            std::shared_ptr<dtk::Timer> timer;
            std::function<void(void)> clickedCallback;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void StatusBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::StatusBar",
                parent);
            DTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.logLabel = dtk::Label::create(context);
            p.logLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.logLabel->setHStretch(dtk::Stretch::Expanding);

            p.infoLabel = dtk::Label::create(context);
            p.infoLabel->setMarginRole(dtk::SizeRole::MarginInside);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.logLabel->setParent(p.layout);
            dtk::Divider::create(context, dtk::Orientation::Horizontal, p.layout);
            p.infoLabel->setParent(p.layout);

            p.timer = dtk::Timer::create(context);

            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    _logUpdate(value);
                });

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& player)
                {
                    DTK_P();
                    std::string text;
                    std::string toolTip;
                    if (player)
                    {
                        const file::Path& path = player->getPath();
                        const io::Info& info = player->getIOInfo();
                        text = play::infoLabel(path, info);
                        toolTip = play::infoToolTip(path, info);
                    }
                    p.infoLabel->setText(text);
                    p.infoLabel->setTooltip(toolTip);;
                });
        }

        StatusBar::StatusBar() :
            _p(new Private)
        {}

        StatusBar::~StatusBar()
        {}

        std::shared_ptr<StatusBar> StatusBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StatusBar>(new StatusBar);
            out->_init(context, app, parent);
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

        void StatusBar::_logUpdate(const std::vector<dtk::LogItem>& value)
        {
            DTK_P();
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case dtk::LogType::Error:
                {
                    const std::string s = dtk::toString(i);
                    p.logLabel->setText(s);
                    p.logLabel->setTooltip(s);
                    p.timer->start(
                        std::chrono::seconds(5),
                        [this]
                        {
                            _p->logLabel->setText(std::string());
                            _p->logLabel->setTooltip(std::string());
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
