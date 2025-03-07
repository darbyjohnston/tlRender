// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ForegroundOptions.h>

#include <dtk/core/ObservableValue.h>

namespace dtk
{
    class Context;
    class Settings;
}

namespace tl
{
    namespace play
    {
        //! Viewport model.
        class ViewportModel : public std::enable_shared_from_this<ViewportModel>
        {
            DTK_NON_COPYABLE(ViewportModel);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            ViewportModel();

        public:
            ~ViewportModel();

            //! Create a new model.
            static std::shared_ptr<ViewportModel> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            //! Get the color picker.
            const dtk::Color4F& getColorPicker() const;

            //! Observe the color picker.
            std::shared_ptr<dtk::IObservableValue<dtk::Color4F> > observeColorPicker() const;

            //! Set the color picker.
            void setColorPicker(const dtk::Color4F&);

            //! Get the image options.
            const dtk::ImageOptions& getImageOptions() const;

            //! Observe the image options.
            std::shared_ptr<dtk::IObservableValue<dtk::ImageOptions> > observeImageOptions() const;

            //! Set the image options.
            void setImageOptions(const dtk::ImageOptions&);

            //! Get the display options.
            const timeline::DisplayOptions& getDisplayOptions() const;

            //! Observe the display options.
            std::shared_ptr<dtk::IObservableValue<timeline::DisplayOptions> > observeDisplayOptions() const;

            //! Set the display options.
            void setDisplayOptions(const timeline::DisplayOptions&);

            //! Get the background options.
            const timeline::BackgroundOptions& getBackgroundOptions() const;

            //! Observe the background options.
            std::shared_ptr<dtk::IObservableValue<timeline::BackgroundOptions> > observeBackgroundOptions() const;

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Get the foreground options.
            const timeline::ForegroundOptions& getForegroundOptions() const;

            //! Observe the foreground options.
            std::shared_ptr<dtk::IObservableValue<timeline::ForegroundOptions> > observeForegroundOptions() const;

            //! Set the foreground options.
            void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Get the color buffer type.
            dtk::ImageType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(dtk::ImageType);

            //! Get whether the HUD is enabled.
            bool getHUD() const;

            //! Observe whether the HUD is enabled.
            std::shared_ptr<dtk::IObservableValue<bool> > observeHUD() const;

            //! Set whether the HUD is enabled.
            void setHUD(bool);

        private:
            DTK_PRIVATE();
        };
    }
}
