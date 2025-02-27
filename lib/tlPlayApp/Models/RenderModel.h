// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/ObservableValue.h>
#include <dtk/core/RenderOptions.h>

namespace dtk
{
    class Context;
    class Settings;
}

namespace tl
{
    namespace play
    {
        //! Render model.
        class RenderModel : public std::enable_shared_from_this<RenderModel>
        {
            DTK_NON_COPYABLE(RenderModel);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            RenderModel();

        public:
            ~RenderModel();

            //! Create a new model.
            static std::shared_ptr<RenderModel> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            //! Get the image options.
            const dtk::ImageOptions& getImageOptions() const;

            //! Observe the image options.
            std::shared_ptr<dtk::IObservableValue<dtk::ImageOptions> > observeImageOptions() const;

            //! Set the image options.
            void setImageOptions(const dtk::ImageOptions&);

            //! Get the color buffer type.
            dtk::ImageType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(dtk::ImageType);

        private:
            DTK_PRIVATE();
        };
    }
}
