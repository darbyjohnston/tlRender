// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/RenderOptions.h>

#include <dtk/core/ObservableValue.h>

namespace dtk
{
    class Context;
}

namespace tl
{
    namespace play
    {
        class Settings;

        //! Render model.
        class RenderModel : public std::enable_shared_from_this<RenderModel>
        {
            TLRENDER_NON_COPYABLE(RenderModel);

        protected:
            void _init(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<dtk::Context>&);

            RenderModel();

        public:
            ~RenderModel();

            //! Create a new model.
            static std::shared_ptr<RenderModel> create(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<dtk::Context>&);

            //! Get the image options.
            const timeline::ImageOptions& getImageOptions() const;

            //! Observe the image options.
            std::shared_ptr<dtk::IObservableValue<timeline::ImageOptions> > observeImageOptions() const;

            //! Set the image options.
            void setImageOptions(const timeline::ImageOptions&);

            //! Get the color buffer type.
            image::PixelType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<dtk::IObservableValue<image::PixelType> > observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(image::PixelType);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
