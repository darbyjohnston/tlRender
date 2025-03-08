// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp//Widgets/Viewport.h>

#include <tlPlayApp/Models/ColorModel.h>
#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/Models/TimeUnitsModel.h>
#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/App.h>

#include <tlTimeline/Util.h>

#include <dtk/ui/ColorSwatch.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/Spacer.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct Viewport::Private
        {
            std::weak_ptr<App> app;
            bool hud = false;
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            double fps = 0.0;
            size_t droppedFrames = 0;
            dtk::Color4F colorPicker;
            dtk::KeyModifier colorPickerModifier = dtk::KeyModifier::None;
            dtk::KeyModifier frameShuttleModifier = dtk::KeyModifier::Shift;

            std::shared_ptr<dtk::Label> currentTimeLabel;
            std::shared_ptr<dtk::Label> fpsLabel;
            std::shared_ptr<dtk::ColorSwatch> colorPickerSwatch;
            std::shared_ptr<dtk::Label> colorPickerLabel;
            std::shared_ptr<dtk::Label> colorBufferLabel;
            std::shared_ptr<dtk::GridLayout> hudLayout;

            enum class MouseMode
            {
                None,
                Shuttle,
                ColorPicker
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                OTIO_NS::RationalTime shuttleStart = time::invalidTime;
            };
            MouseData mouse;

            std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<dtk::ValueObserver<double> > fpsObserver;
            std::shared_ptr<dtk::ValueObserver<size_t> > droppedFramesObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::Color4F> > colorPickerObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > bgOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::ForegroundOptions> > fgOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > hudObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
            std::shared_ptr<dtk::ValueObserver<MouseSettings> > mouseSettingsObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            timelineui::Viewport::_init(context, parent);
            DTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.app = app;

            p.currentTimeLabel = dtk::Label::create(context);
            p.currentTimeLabel->setFontRole(dtk::FontRole::Mono);
            p.currentTimeLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.currentTimeLabel->setBackgroundRole(dtk::ColorRole::Overlay);

            p.fpsLabel = dtk::Label::create(context);
            p.fpsLabel->setFontRole(dtk::FontRole::Mono);
            p.fpsLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.fpsLabel->setBackgroundRole(dtk::ColorRole::Overlay);

            p.colorPickerSwatch = dtk::ColorSwatch::create(context);
            p.colorPickerSwatch->setSizeRole(dtk::SizeRole::MarginLarge);
            p.colorPickerLabel = dtk::Label::create(context);
            p.colorPickerLabel->setFontRole(dtk::FontRole::Mono);
            p.colorPickerLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.colorPickerLabel->setBackgroundRole(dtk::ColorRole::Overlay);

            p.colorBufferLabel = dtk::Label::create(context);
            p.colorBufferLabel->setFontRole(dtk::FontRole::Mono);
            p.colorBufferLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.colorBufferLabel->setBackgroundRole(dtk::ColorRole::Overlay);

            p.hudLayout = dtk::GridLayout::create(context, shared_from_this());
            p.hudLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.hudLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.currentTimeLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.currentTimeLabel, 0, 0);
            p.fpsLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.fpsLabel, 0, 2);
            auto spacer = dtk::Spacer::create(context, dtk::Orientation::Horizontal, p.hudLayout);
            spacer->setStretch(dtk::Stretch::Expanding, dtk::Stretch::Expanding);
            p.hudLayout->setGridPos(spacer, 1, 1);
            auto hLayout = dtk::HorizontalLayout::create(context, p.hudLayout);
            p.hudLayout->setGridPos(hLayout, 2, 0);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.colorPickerSwatch->setParent(hLayout);
            p.colorPickerLabel->setParent(hLayout);
            p.colorBufferLabel->setParent(p.hudLayout);
            p.hudLayout->setGridPos(p.colorBufferLabel, 2, 2);
            p.hudLayout->hide();

            p.fpsObserver = dtk::ValueObserver<double>::create(
                observeFPS(),
                [this](double value)
                {
                    _p->fps = value;
                    _hudUpdate();
                });

            p.droppedFramesObserver = dtk::ValueObserver<size_t>::create(
                observeDroppedFrames(),
                [this](size_t value)
                {
                    _p->droppedFrames = value;
                    _hudUpdate();
                });

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    setCompareOptions(value);
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->getColorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                   setOCIOOptions(value);
                });

            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->getColorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                   setLUTOptions(value);
                });

            p.colorPickerObserver = dtk::ValueObserver<dtk::Color4F>::create(
                app->getViewportModel()->observeColorPicker(),
                [this](const dtk::Color4F& value)
                {
                    _p->colorPicker = value;
                    _hudUpdate();
                });

            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                app->getViewportModel()->observeImageOptions(),
                [this](const dtk::ImageOptions& value)
                {
                   setImageOptions({ value });
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                   setDisplayOptions({ value });
                });

            p.bgOptionsObserver = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                app->getViewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    setBackgroundOptions(value);
                });

            p.fgOptionsObserver = dtk::ValueObserver<timeline::ForegroundOptions>::create(
                app->getViewportModel()->observeForegroundOptions(),
                [this](const timeline::ForegroundOptions& value)
                {
                    setForegroundOptions(value);
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->getViewportModel()->observeColorBuffer(),
                [this](dtk::ImageType value)
                {
                    setColorBuffer(value);
                    _hudUpdate();
                });

            p.hudObserver = dtk::ValueObserver<bool>::create(
                app->getViewportModel()->observeHUD(),
                [this](bool value)
                {
                    _p->hud = value;
                    _hudUpdate();
                });

            p.timeUnitsObserver = dtk::ValueObserver<timeline::TimeUnits>::create(
                app->getTimeUnitsModel()->observeTimeUnits(),
                [this](timeline::TimeUnits value)
                {
                    _hudUpdate();
                });

            p.mouseSettingsObserver = dtk::ValueObserver<MouseSettings>::create(
                app->getSettingsModel()->observeMouse(),
                [this](const MouseSettings& value)
                {
                    auto i = value.actions.find(MouseAction::PanView);
                    setPanModifier(i != value.actions.end() ? i->second : dtk::KeyModifier::None);
                    i = value.actions.find(MouseAction::CompareWipe);
                    setWipeModifier(i != value.actions.end() ? i->second : dtk::KeyModifier::None);
                    i = value.actions.find(MouseAction::ColorPicker);
                    _p->colorPickerModifier = i != value.actions.end() ? i->second : dtk::KeyModifier::None;
                    i = value.actions.find(MouseAction::FrameShuttle);
                    _p->frameShuttleModifier = i != value.actions.end() ? i->second : dtk::KeyModifier::None;
                });
        }

        Viewport::Viewport() :
            _p(new Private)
        {}

        Viewport::~Viewport()
        {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Viewport>(new Viewport);
            out->_init(context, app, parent);
            return out;
        }

        void Viewport::setPlayer(const std::shared_ptr<timeline::Player>& player)
        {
            timelineui::Viewport::setPlayer(player);
            DTK_P();
            if (player)
            {
                p.currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                    player->observeCurrentTime(),
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        _p->currentTime = value;
                        _hudUpdate();
                    });
            }
            else
            {
                p.currentTimeObserver.reset();
            }
        }

        void Viewport::setGeometry(const dtk::Box2I& value)
        {
            timelineui::Viewport::setGeometry(value);
            DTK_P();
            p.hudLayout->setGeometry(value);
        }

        void Viewport::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            timelineui::Viewport::sizeHintEvent(event);
            DTK_P();
            _setSizeHint(p.hudLayout->getSizeHint());
        }

        void Viewport::mouseMoveEvent(dtk::MouseMoveEvent& event)
        {
            timelineui::Viewport::mouseMoveEvent(event);
            DTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Shuttle:
                if (auto player = getPlayer())
                {
                    const OTIO_NS::RationalTime offset = OTIO_NS::RationalTime(
                        (event.pos.x - _getMousePressPos().x) * .05F,
                        p.mouse.shuttleStart.rate()).round();
                    const OTIO_NS::TimeRange& timeRange = player->getTimeRange();
                    OTIO_NS::RationalTime t = p.mouse.shuttleStart + offset;
                    if (t < timeRange.start_time())
                    {
                        t = timeRange.end_time_exclusive() - (timeRange.start_time() - t);
                    }
                    else if (t > timeRange.end_time_exclusive())
                    {
                        t = timeRange.start_time() + (t - timeRange.end_time_exclusive());
                    }
                    player->seek(t);
                }
                break;
            case Private::MouseMode::ColorPicker:
                if (auto app = p.app.lock())
                {
                    const dtk::Color4F color = getColorSample(event.pos);
                    app->getViewportModel()->setColorPicker(color);
                }
                break;
            default: break;
            }
        }

        void Viewport::mousePressEvent(dtk::MouseClickEvent& event)
        {
            timelineui::Viewport::mousePressEvent(event);
            DTK_P();
            takeKeyFocus();
            if (0 == event.button &&
                dtk::checkKeyModifier(p.colorPickerModifier, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::ColorPicker;
                if (auto app = p.app.lock())
                {
                    const dtk::Color4F color = getColorSample(event.pos);
                    app->getViewportModel()->setColorPicker(color);
                }
            }
            else if (0 == event.button &&
                dtk::checkKeyModifier(p.frameShuttleModifier, event.modifiers))
            {
                p.mouse.mode = Private::MouseMode::Shuttle;
                if (auto player = getPlayer())
                {
                    player->stop();
                    p.mouse.shuttleStart = player->getCurrentTime();
                }
            }
        }

        void Viewport::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            timelineui::Viewport::mouseReleaseEvent(event);
            DTK_P();
            p.mouse = Private::MouseData();
        }

        void Viewport::_hudUpdate()
        {
            DTK_P();

            std::string s;
            if (auto app = p.app.lock())
            {
                auto timeUnitsModel = app->getTimeUnitsModel();
                s = timeUnitsModel->getLabel(p.currentTime);
            }
            p.currentTimeLabel->setText(
                dtk::Format("Current time: {0}").
                arg(s));

            p.fpsLabel->setText(
                dtk::Format("FPS: {0} ({1} dropped)").
                arg(p.fps, 2, 4).
                arg(p.droppedFrames));

            p.colorPickerSwatch->setColor(p.colorPicker);
            p.colorPickerLabel->setText(
                dtk::Format("Color picker: {0} {1} {2} {3}").
                arg(p.colorPicker.r, 2).
                arg(p.colorPicker.g, 2).
                arg(p.colorPicker.b, 2).
                arg(p.colorPicker.a, 2));

            p.colorBufferLabel->setText(
                dtk::Format("Color buffer: {0}").
                arg(getColorBuffer()));

            p.hudLayout->setVisible(p.hud);
        }
    }
}
