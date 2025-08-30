// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include "RecentFilesModel.h"

namespace tl
{
    namespace play
    {
        void RecentFilesModel::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::Settings>& settings)
        {
            ftk::RecentFilesModel::_init(context);

            _settings = settings;

            std::vector<std::filesystem::path> recent;
            nlohmann::json json;
            if (_settings->get("/Files/Recent", json))
            {
                for (auto i = json.begin(); i != json.end(); ++i)
                {
                    if (i->is_string())
                    {
                        recent.push_back(std::filesystem::u8path(i->get<std::string>()));
                    }
                }
            }
            setRecent(recent);
            size_t max = 10;
            _settings->get("/Files/RecentMax", max);
            setRecentMax(max);
        }

        RecentFilesModel::~RecentFilesModel()
        {
            nlohmann::json json;
            for (const auto& path : getRecent())
            {
                json.push_back(path.u8string());
            }
            _settings->set("/Files/Recent", json);
            _settings->set("/Files/RecentMax", getRecentMax());
        }

        std::shared_ptr<RecentFilesModel> RecentFilesModel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::Settings>& settings)
        {
            auto out = std::shared_ptr<RecentFilesModel>(new RecentFilesModel);
            out->_init(context, settings);
            return out;
        }
    }
}