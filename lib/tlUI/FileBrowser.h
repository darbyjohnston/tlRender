// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IDialog.h>

#include <tlCore/FileInfo.h>
#include <tlCore/ISystem.h>

namespace tl
{
    namespace ui
    {
        class RecentFilesModel;

        //! File browser options.
        struct FileBrowserOptions
        {
            std::string    search;
            std::string    extension;
            file::ListSort sort            = file::ListSort::Name;
            bool           reverseSort     = false;
            bool           sequence        = true;
            bool           thumbnails      = true;
            int            thumbnailHeight = 100;

            bool operator == (const FileBrowserOptions&) const;
            bool operator != (const FileBrowserOptions&) const;
        };

        //! File browser.
        class FileBrowser : public IDialog
        {
            TLRENDER_NON_COPYABLE(FileBrowser);

        protected:
            void _init(
                const std::string& path,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FileBrowser();

        public:
            virtual ~FileBrowser();

            //! Create a new dialog.
            static std::shared_ptr<FileBrowser> create(
                const std::string& path,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the callback.
            void setCallback(const std::function<void(const file::FileInfo&)>&);

            //! Get the path.
            const std::string& getPath() const;

            //! Get the options.
            const FileBrowserOptions& getOptions() const;

            //! Set the options.
            void setOptions(const FileBrowserOptions&);

            //! Set the recent files model.
            void setRecentFilesModel(const std::shared_ptr<RecentFilesModel>&);

        private:
            TLRENDER_PRIVATE();
        };

        //! File browser system.
        class FileBrowserSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(FileBrowserSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            FileBrowserSystem();

        public:
            virtual ~FileBrowserSystem();

            //! Create a new system.
            static std::shared_ptr<FileBrowserSystem> create(const std::shared_ptr<system::Context>&);

            //! Open the file browser.
            void open(
                const std::shared_ptr<EventLoop>&,
                const std::function<void(const file::FileInfo&)>&);

            //! Get whether the native file dialog is used.
            bool isNativeFileDialog() const;

            //! Set whether the native file dialog is used.
            void setNativeFileDialog(bool);

            //! Get the path.
            const std::string& getPath() const;

            //! Set the path.
            void setPath(const std::string&);

            //! Get the options.
            const FileBrowserOptions& getOptions() const;

            //! Set the options.
            void setOptions(const FileBrowserOptions&);

            //! Get the recent files model.
            const std::shared_ptr<RecentFilesModel>& getRecentFilesModel() const;

        private:
            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const FileBrowserOptions&);

        void from_json(const nlohmann::json&, FileBrowserOptions&);

        ///@}
    }
}
