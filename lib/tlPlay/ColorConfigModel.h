// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ColorConfigOptions.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Color configuration model data.
        struct ColorConfigModelData
        {
            bool enabled = false;
            std::string fileName;
            std::vector<std::string> inputs;
            size_t inputIndex = 0;
            std::vector<std::string> displays;
            size_t displayIndex = 0;
            std::vector<std::string> views;
            size_t viewIndex = 0;

            bool operator == (const ColorConfigModelData&) const;
            bool operator != (const ColorConfigModelData&) const;
        };

        //! Color configuration model.
        class ColorConfigModel : public std::enable_shared_from_this<ColorConfigModel>
        {
            TLRENDER_NON_COPYABLE(ColorConfigModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ColorConfigModel();

        public:
            ~ColorConfigModel();

            //! Create a new model.
            static std::shared_ptr<ColorConfigModel> create(const std::shared_ptr<system::Context>&);

            //! Observe the color configuration options.
            std::shared_ptr<observer::IValue<timeline::ColorConfigOptions> > observeConfigOptions() const;

            //! Set the color configuration options.
            void setConfigOptions(const timeline::ColorConfigOptions&);

            //! Set whether the color configuration is enabled.
            void setEnabled(bool);

            //! Set the color configuration.
            void setConfig(const std::string& fileName);

            //! Observe the model data.
            std::shared_ptr<observer::IValue<ColorConfigModelData> > observeData() const;

            //! Set the input index.
            void setInputIndex(size_t);

            //! Set the display index.
            void setDisplayIndex(size_t);

            //! Set the view index.
            void setViewIndex(size_t);

        private:
            void _configUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
