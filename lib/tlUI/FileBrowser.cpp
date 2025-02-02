// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

namespace tl
{
    namespace ui
    {
        bool FileBrowserOptions::operator == (const FileBrowserOptions& other) const
        {
            return
                search == other.search &&
                extension == other.extension &&
                sort == other.sort &&
                reverseSort == other.reverseSort &&
                sequence == other.sequence &&
                thumbnails == other.thumbnails &&
                thumbnailHeight == other.thumbnailHeight;
        }

        bool FileBrowserOptions::operator != (const FileBrowserOptions& other) const
        {
            return !(*this == other);
        }

        struct FileBrowser::Private
        {
            std::shared_ptr<FileBrowserWidget> widget;
        };

        void FileBrowser::_init(
            const std::string& path,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IDialog::_init("tl::ui::FileBrowser", context, parent);
            TLRENDER_P();

            p.widget = FileBrowserWidget::create(
                path,
                context,
                shared_from_this());

            p.widget->setCancelCallback(
                [this]
                {
                    close();
                });
        }

        FileBrowser::FileBrowser() :
            _p(new Private)
        {}

        FileBrowser::~FileBrowser()
        {}

        std::shared_ptr<FileBrowser> FileBrowser::create(
            const std::string& path,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowser>(new FileBrowser);
            out->_init(path, context, parent);
            return out;
        }

        void FileBrowser::setCallback(const std::function<void(const file::FileInfo&)>& value)
        {
            _p->widget->setCallback(value);
        }

        const std::string& FileBrowser::getPath() const
        {
            return _p->widget->getPath();
        }

        const FileBrowserOptions& FileBrowser::getOptions() const
        {
            return _p->widget->getOptions();
        }

        void FileBrowser::setOptions(const FileBrowserOptions& value)
        {
            _p->widget->setOptions(value);
        }

        void FileBrowser::setRecentFilesModel(const std::shared_ptr<RecentFilesModel>& value)
        {
            _p->widget->setRecentFilesModel(value);
        }

        void to_json(nlohmann::json& json, const FileBrowserOptions& value)
        {
            json =
            {
                { "search", value.search },
                { "extension", value.extension },
                { "sort", value.sort },
                { "reverseSort", value.reverseSort },
                { "sequence", value.sequence },
                { "thumbnails", value.thumbnails },
                { "thumbnailHeight", value.thumbnailHeight }
            };
        }

        void from_json(const nlohmann::json& json, FileBrowserOptions& value)
        {
            json.at("search").get_to(value.search);
            json.at("extension").get_to(value.extension);
            json.at("sort").get_to(value.sort);
            json.at("reverseSort").get_to(value.reverseSort);
            json.at("sequence").get_to(value.sequence);
            json.at("thumbnails").get_to(value.thumbnails);
            json.at("thumbnailHeight").get_to(value.thumbnailHeight);
        }
    }
}
