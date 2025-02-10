// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/DisplayOptions.h>

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

        private:
            DTK_PRIVATE();
        };
    }
}
