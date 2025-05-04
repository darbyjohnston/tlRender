// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <dtk/ui/Settings.h>

namespace tl
{
    namespace play
    {
        //! Settings model.
        class SettingsModel : public std::enable_shared_from_this<SettingsModel>
        {
            DTK_NON_COPYABLE(SettingsModel);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::filesystem::path&);

            SettingsModel() = default;

        public:
            ~SettingsModel();

            //! Create a new model.
            static std::shared_ptr<SettingsModel> create(
                const std::shared_ptr<dtk::Context>&,
                const std::filesystem::path&);

            //! Get the settings.
            const std::shared_ptr<dtk::Settings>& getSettings() const;

            //! Get the cache settings.
            const timeline::PlayerCacheOptions& getCache() const;

            //! Observe the cache settings.
            std::shared_ptr<dtk::IObservableValue<timeline::PlayerCacheOptions> > observeCache() const;

            //! Set the cache settings.
            void setCache(const timeline::PlayerCacheOptions&);

        private:
            std::shared_ptr<dtk::Settings> _settings;
            std::shared_ptr<dtk::ObservableValue<timeline::PlayerCacheOptions> > _cache;
        };
    }
}