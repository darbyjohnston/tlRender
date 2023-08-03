// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/RecentFilesModel.h>

namespace tl
{
    namespace ui
    {
        struct RecentFilesModel::Private
        {
            size_t recentMax = 10;
            std::shared_ptr<observer::List<file::Path> > recent;
        };

        void RecentFilesModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.recent = observer::List<file::Path>::create();
        }

        RecentFilesModel::RecentFilesModel() :
            _p(new Private)
        {}

        RecentFilesModel::~RecentFilesModel()
        {}

        std::shared_ptr<RecentFilesModel> RecentFilesModel::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<RecentFilesModel>(new RecentFilesModel);
            out->_init(context);
            return out;
        }

        size_t RecentFilesModel::getRecentMax() const
        {
            return _p->recentMax;
        }

        void RecentFilesModel::setRecentMax(size_t value)
        {
            TLRENDER_P();
            if (p.recentMax == value)
                return;
            p.recentMax = value;
            if (p.recent->getSize() > p.recentMax)
            {
                auto recent = p.recent->get();
                while (recent.size() > p.recentMax)
                {
                    recent.erase(recent.begin());
                }
                p.recent->setIfChanged(recent);
            }
        }

        const std::vector<file::Path>& RecentFilesModel::getRecent() const
        {
            return _p->recent->get();
        }

        std::shared_ptr<observer::IList<file::Path> > RecentFilesModel::observeRecent() const
        {
            return _p->recent;
        }

        void RecentFilesModel::setRecent(const std::vector<file::Path>& value)
        {
            TLRENDER_P();
            auto recent = value;
            while (recent.size() > p.recentMax)
            {
                recent.erase(recent.begin());
            }
            p.recent->setIfChanged(recent);
        }

        void RecentFilesModel::addRecent(const file::Path& value)
        {
            TLRENDER_P();
            auto recent = p.recent->get();
            auto i = recent.begin();
            while (i != recent.end())
            {
                if (*i == value)
                {
                    i = recent.erase(i);
                }
                else
                {
                    ++i;
                }
            }
            recent.push_back(value);
            while (recent.size() > p.recentMax)
            {
                recent.erase(recent.begin());
            }
            p.recent->setIfChanged(recent);
        }
    }
}