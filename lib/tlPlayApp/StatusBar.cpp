// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/StatusBar.h>

#include <tlPlayApp/App.h>

#include <tlPlay/Info.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <dtk/ui/Divider.h>
#include <dtk/ui/Icon.h>
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
#if defined(TLRENDER_BMD)
            std::shared_ptr<dtk::Icon> deviceActiveIcon;
#endif // TLRENDER_BMD
            std::shared_ptr<dtk::HorizontalLayout> layout;

            std::shared_ptr<dtk::Timer> timer;
            std::function<void(void)> clickedCallback;

            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
#if defined(TLRENDER_BMD)
            std::shared_ptr<dtk::ValueObserver<bool> > bmdActiveObserver;
#endif // TLRENDER_BMD
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

#if defined(TLRENDER_BMD)
            p.deviceActiveIcon = dtk::Icon::create(context, "Devices");
            p.deviceActiveIcon->setTooltip("Output device active");
#endif // TLRENDER_BMD

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.logLabel->setParent(p.layout);
            dtk::Divider::create(context, dtk::Orientation::Horizontal, p.layout);
            p.infoLabel->setParent(p.layout);
#if defined(TLRENDER_BMD)
            dtk::Divider::create(context, dtk::Orientation::Horizontal, p.layout);
            p.deviceActiveIcon->setParent(p.layout);
#endif // TLRENDER_BMD

            _deviceUpdate(false);

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
                    _infoUpdate(
                        player ? player->getPath() : file::Path(),
                        player ? player->getIOInfo() : io::Info());
                });

#if defined(TLRENDER_BMD)
            p.bmdActiveObserver = dtk::ValueObserver<bool>::create(
                app->getBMDOutputDevice()->observeActive(),
                [this](bool value)
                {
                    _deviceUpdate(value);
                });
#endif // TLRENDER_BMD
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

        void StatusBar::_infoUpdate(const file::Path& path, const io::Info& info)
        {
            DTK_P();
            const std::string text = play::infoLabel(path, info);
            const std::string toolTip = play::infoToolTip(path, info);
            p.infoLabel->setText(text);
            p.infoLabel->setTooltip(toolTip);;
        }

        void StatusBar::_deviceUpdate(bool value)
        {
            DTK_P();
#if defined(TLRENDER_BMD)
            p.deviceActiveIcon->setEnabled(value);
            p.deviceActiveIcon->setBackgroundRole(value ? dtk::ColorRole::Checked : dtk::ColorRole::None);
#endif // TLRENDER_BMD
        }
    }
}
