// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ListObserver.h>
#include <tlCore/ValueObserver.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace ui
    {
        //! Recent files model.
        class RecentFilesModel : public std::enable_shared_from_this<RecentFilesModel>
        {
            TLRENDER_NON_COPYABLE(RecentFilesModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            RecentFilesModel();

        public:
            ~RecentFilesModel();

            //! Create a new model.
            static std::shared_ptr<RecentFilesModel> create(
                const std::shared_ptr<system::Context>&);

            //! Get the maximum number of recent files.
            size_t getRecentMax() const;

            //! Observe the maximum number of recent files.
            std::shared_ptr<observer::IValue<size_t> > observeRecentMax() const;

            //! Set the maximum number of recent files.
            void setRecentMax(size_t);

            //! Get the list of recent files.
            const std::vector<file::Path>& getRecent() const;

            //! Observe the list of recent files.
            std::shared_ptr<observer::IList<file::Path> > observeRecent() const;

            //! Set the recent files.
            void setRecent(const std::vector<file::Path>&);

            //! Add a recent file.
            void addRecent(const file::Path&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
