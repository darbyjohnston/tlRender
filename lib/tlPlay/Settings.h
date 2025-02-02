// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <dtk/core/ObservableValue.h>

#include <nlohmann/json.hpp>

#include <string>

namespace dtk
{
    class Context;
}

namespace tl
{
    namespace play
    {
        class App;

        //! Settings.
        class Settings : public std::enable_shared_from_this<Settings>
        {
            TLRENDER_NON_COPYABLE(Settings);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::string fileName,
                bool reset);

            Settings();

        public:
            virtual ~Settings();

            //! Create a new settings.
            static std::shared_ptr<Settings> create(
                const std::shared_ptr<dtk::Context>&,
                const std::string fileName,
                bool reset);

            //! Get a value.
            template<typename T>
            T getValue(const std::string&) const;

            //! Observe value changes.
            std::shared_ptr<dtk::IObservableValue<std::string> > observeValues() const;

            //! Set a default value.
            template<typename T>
            void setDefaultValue(const std::string&, T);

            //! Set a value.
            template<typename T>
            void setValue(const std::string&, T);

            //! Reset the settings to defaults.
            void reset();

        private:
            void _read();
            void _write();

            std::weak_ptr<dtk::Context> _context;
            std::string _fileName;
            nlohmann::json _defaultValues;
            nlohmann::json _values;
            std::shared_ptr<dtk::ObservableValue<std::string> > _observer;
        };
    }
}

#include <tlPlay/SettingsInline.h>
