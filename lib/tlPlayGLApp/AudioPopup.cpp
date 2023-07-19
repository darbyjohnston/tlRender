// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/AudioPopup.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>

#include <tlUI/IntEditSlider.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        struct AudioPopup::Private
        {
            std::shared_ptr<ui::IntEditSlider> volumeSlider;
            std::shared_ptr<ui::ToolButton> muteButton;
            std::shared_ptr<ui::HorizontalLayout> layout;
            std::shared_ptr<observer::ValueObserver<int> > volumeObserver;
            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IMenuPopup::_init(
                "tl::play_gl::AudioPopup",
                context,
                parent);
            TLRENDER_P();

            p.volumeSlider = ui::IntEditSlider::create(context);
            p.volumeSlider->getModel()->setRange(math::IntRange(0, 100));
            p.volumeSlider->getModel()->setStep(1);
            p.volumeSlider->getModel()->setLargeStep(10);

            p.muteButton = ui::ToolButton::create(context);
            p.muteButton->setCheckable(true);
            p.muteButton->setIcon("Mute");

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginInside);
            p.layout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.volumeSlider->setParent(p.layout);
            p.muteButton->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.volumeObserver = observer::ValueObserver<int>::create(
                p.volumeSlider->getModel()->observeValue(),
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        std::stringstream ss;
                        ss << value / 100.0;
                        app->getSettings()->setData("Volume", ss.str());
                    }
                },
                observer::CallbackAction::Suppress);

            p.muteButton->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        std::stringstream ss;
                        ss << value;
                        app->getSettings()->setData("Mute", ss.str());
                    }
                });

            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeData(),
                [this](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    auto i = value.find("Volume");
                    if (i != value.end())
                    {
                        const double volume = std::atof(i->second.c_str());
                        p.volumeSlider->getModel()->setValue(volume * 100.0);
                    }
                    i = value.find("Mute");
                    if (i != value.end())
                    {
                        const bool mute = std::atoi(i->second.c_str());
                        p.muteButton->setChecked(mute);
                    }
                });
        }

        AudioPopup::AudioPopup() :
            _p(new Private)
        {}

        AudioPopup::~AudioPopup()
        {}

        std::shared_ptr<AudioPopup> AudioPopup::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioPopup>(new AudioPopup);
            out->_init(app, context, parent);
            return out;
        }
    }
}
