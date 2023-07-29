// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

namespace tl
{
    namespace ui
    {
        bool FileBrowserOptions::operator == (const FileBrowserOptions& other) const
        {
            return filter == other.filter &&
                extension == other.extension &&
                list == other.list;
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
            const std::shared_ptr<system::Context>& context,
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
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowser>(new FileBrowser);
            out->_init(path, context, parent);
            return out;
        }

        void FileBrowser::setFileCallback(const std::function<void(const file::Path&)>& value)
        {
            _p->widget->setFileCallback(value);
        }

        const FileBrowserOptions& FileBrowser::getOptions() const
        {
            return _p->widget->getOptions();
        }

        void FileBrowser::setOptions(const FileBrowserOptions& value)
        {
            _p->widget->setOptions(value);
        }

        void to_json(nlohmann::json& json, const FileBrowserOptions& value)
        {
            nlohmann::json list;
            to_json(list, value.list);
            json =
            {
                { "filter", value.filter },
                { "extension", value.extension },
                { "list", list }
            };
        }

        void from_json(const nlohmann::json& json, FileBrowserOptions& value)
        {
            json.at("filter").get_to(value.filter);
            json.at("extension").get_to(value.extension);
            from_json(json.at("list"), value.list);
        }
    }
}
