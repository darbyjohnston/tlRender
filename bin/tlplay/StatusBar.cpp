// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "StatusBar.h"

#include "App.h"

namespace tl
{
    namespace play
    {
        void StatusBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "StatusBar", parent);

            _logLabel = ftk::Label::create(context, shared_from_this());
            
            _logTimer = ftk::Timer::create(context);

            _logObserver = ftk::ListObserver<ftk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<ftk::LogItem>& value)
                {
                    _logUpdate(value);
                });
        }

        StatusBar::~StatusBar()
        {}

        std::shared_ptr<StatusBar> StatusBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StatusBar>(new StatusBar);
            out->_init(context, app, parent);
            return out;
        }

        void StatusBar::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _logLabel->setGeometry(value);
        }

        void StatusBar::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_logLabel->getSizeHint());
        }

        void StatusBar::_logUpdate(const std::vector<ftk::LogItem>& value)
        {
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case ftk::LogType::Error:
                {
                    const std::string s = ftk::toString(i);
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