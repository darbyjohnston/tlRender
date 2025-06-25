// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "StatusBar.h"

#include "App.h"

namespace tl
{
    namespace play
    {
        void StatusBar::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "StatusBar", parent);

            _logLabel = feather_tk::Label::create(context, shared_from_this());
            
            _logTimer = feather_tk::Timer::create(context);

            _logObserver = feather_tk::ListObserver<feather_tk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<feather_tk::LogItem>& value)
                {
                    _logUpdate(value);
                });
        }

        StatusBar::~StatusBar()
        {}

        std::shared_ptr<StatusBar> StatusBar::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StatusBar>(new StatusBar);
            out->_init(context, app, parent);
            return out;
        }

        void StatusBar::setGeometry(const feather_tk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _logLabel->setGeometry(value);
        }

        void StatusBar::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_logLabel->getSizeHint());
        }

        void StatusBar::_logUpdate(const std::vector<feather_tk::LogItem>& value)
        {
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case feather_tk::LogType::Error:
                {
                    const std::string s = feather_tk::toString(i);
                    _logLabel->setText(s);
                    _logLabel->setTooltip(s);
                    _logTimer->start(
                        std::chrono::seconds(5),
                        [this]
                        {
                            _logLabel->setText(std::string());
                            _logLabel->setTooltip(std::string());
                        });
                    break;
                }
                default: break;
                }
            }
        }
    }
}