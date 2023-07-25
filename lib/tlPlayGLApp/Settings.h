// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/MapObserver.h>

#include <string>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play_gl
    {
        class App;

        //! Audio tool.
        class Settings : public std::enable_shared_from_this<Settings>
        {
            TLRENDER_NON_COPYABLE(Settings);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Settings();

        public:
            virtual ~Settings();

            //! Create a new settings.
            static std::shared_ptr<Settings> create(
                const std::shared_ptr<system::Context>&);

            //! Get the settings data.
            const std::map<std::string, std::string>& getData() const;

            //! Get the settings data.
            std::string getData(const std::string&) const;

            //! Get the settings data.
            template<typename T>
            T getData(const std::string&) const;

            //! Observe the settings data.
            std::shared_ptr<observer::IMap<std::string, std::string> > observeData() const;

            //! Set the settings data.
            void setData(const std::string&, const std::string&);

            //! Set the settings data.
            template<typename T>
            void setData(const std::string&, T);

        private:
            TLRENDER_PRIVATE();
        };
    }
}

#include <tlPlayGLApp/SettingsInline.h>
