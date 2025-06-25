// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include "SettingsModel.h"

namespace tl
{
    namespace play
    {
        void SettingsModel::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::filesystem::path& path)
        {
            _settings = feather_tk::Settings::create(context, path);

            timeline::PlayerCacheOptions cache;
            _settings->getT("/Cache", cache);
            _cache = feather_tk::ObservableValue<timeline::PlayerCacheOptions>::create(cache);
        }

        SettingsModel::~SettingsModel()
        {
            _settings->setT("/Cache", _cache->get());
        }

        std::shared_ptr<SettingsModel> SettingsModel::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::filesystem::path& path)
        {
            auto out = std::shared_ptr<SettingsModel>(new SettingsModel);
            out->_init(context, path);
            return out;
        }

        const std::shared_ptr<feather_tk::Settings>& SettingsModel::getSettings() const
        {
            return _settings;
        }

        const timeline::PlayerCacheOptions& SettingsModel::getCache() const
        {
            return _cache->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<timeline::PlayerCacheOptions> > SettingsModel::observeCache() const
        {
            return _cache;
        }

        void SettingsModel::setCache(const timeline::PlayerCacheOptions& value)
        {
            _cache->setIfChanged(value);
        }
    }
}