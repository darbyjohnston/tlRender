// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/FileBrowser.h>

#include <tlUI/IButton.h>

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace ui
    {
        class PathsWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(PathsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            PathsWidget();

        public:
            virtual ~PathsWidget();

            static std::shared_ptr<PathsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<void(const std::string&)>&);

            void setRecentFilesModel(const std::shared_ptr<RecentFilesModel>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _createButton(
                const std::string&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            void _pathsUpdate();

            TLRENDER_PRIVATE();
        };

        class Button : public IButton
        {
            TLRENDER_NON_COPYABLE(Button);

        protected:
            void _init(
                const file::FileInfo&,
                const FileBrowserOptions&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Button();

        public:
            virtual ~Button();

            static std::shared_ptr<Button> create(
                const file::FileInfo&,
                const FileBrowserOptions&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            const std::vector<int>& getTextWidths() const;

            void setColumns(const std::vector<int>&);

            void tickEvent(bool, bool, const TickEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const DrawEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };

        class DirectoryWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(DirectoryWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            DirectoryWidget();

        public:
            virtual ~DirectoryWidget();

            static std::shared_ptr<DirectoryWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setPath(const std::string&);

            void reload();

            void setCallback(const std::function<void(const file::FileInfo&)>&);

            void setOptions(const FileBrowserOptions&);

            const FileBrowserOptions& getOptions() const;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _directoryUpdate();

            TLRENDER_PRIVATE();
        };

        class FileBrowserWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(FileBrowserWidget);

        protected:
            void _init(
                const std::string&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FileBrowserWidget();

        public:
            virtual ~FileBrowserWidget();

            static std::shared_ptr<FileBrowserWidget> create(
                const std::string&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<void(const file::FileInfo&)>&);

            void setCancelCallback(const std::function<void(void)>&);

            const std::string& getPath() const;

            const FileBrowserOptions& getOptions() const;

            void setOptions(const FileBrowserOptions&);
            
            void setOptionsCallback(const std::function<void(const FileBrowserOptions&)>&);

            void setRecentFilesModel(const std::shared_ptr<RecentFilesModel>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            void _pathUpdate();
            void _optionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
