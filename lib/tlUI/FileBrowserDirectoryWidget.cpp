// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/RowLayout.h>

#include <tlCore/String.h>

namespace tl
{
    namespace ui
    {
        struct DirectoryWidget::Private
        {
            std::string path;
            FileBrowserOptions options;
            std::vector<file::FileInfo> fileInfos;
            std::vector<std::shared_ptr<Button> > buttons;
            std::map<std::shared_ptr<Button>, size_t> buttonToIndex;
            std::shared_ptr<ButtonGroup> buttonGroup;
            std::shared_ptr<VerticalLayout> layout;
            std::function<void(const std::string&)> fileCallback;
            std::function<void(const std::string&)> pathCallback;

            struct SizeData
            {
                int spacing = 0;
            };
            SizeData size;
        };

        void DirectoryWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::DirectoryWidget", context, parent);
            TLRENDER_P();

            setBackgroundRole(ColorRole::Base);

            p.options.list.sequence = false;

            p.buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);

            p.buttonGroup->setClickedCallback(
                [this](int value)
                {
                    TLRENDER_P();
                    if (value >= 0 &&
                        value < p.buttons.size())
                    {
                        const auto i = p.buttonToIndex.find(p.buttons[value]);
                        if (i != p.buttonToIndex.end())
                        {
                            const file::FileInfo& fileInfo = p.fileInfos[i->second];
                            switch (fileInfo.getType())
                            {
                            case file::Type::File:
                                if (p.fileCallback)
                                {
                                    p.fileCallback(fileInfo.getPath().get(-1, false));
                                }
                                break;
                            case file::Type::Directory:
                            {
                                p.path = file::Path(p.path, fileInfo.getPath().get(-1, false)).get();
                                _directoryUpdate();
                                if (p.pathCallback)
                                {
                                    p.pathCallback(p.path);
                                }
                                break;
                            }
                            default: break;
                            }
                        }
                    }
                });
        }

        DirectoryWidget::DirectoryWidget() :
            _p(new Private)
        {}

        DirectoryWidget::~DirectoryWidget()
        {}

        std::shared_ptr<DirectoryWidget> DirectoryWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DirectoryWidget>(new DirectoryWidget);
            out->_init(context, parent);
            return out;
        }

        void DirectoryWidget::setPath(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.path)
                return;
            p.path = value;
            _directoryUpdate();
        }

        void DirectoryWidget::setFileCallback(const std::function<void(const std::string&)>& value)
        {
            _p->fileCallback = value;
        }

        void DirectoryWidget::setPathCallback(const std::function<void(const std::string&)>& value)
        {
            _p->pathCallback = value;
        }

        void DirectoryWidget::setOptions(const FileBrowserOptions& value)
        {
            TLRENDER_P();
            if (value == p.options)
                return;
            p.options = value;
            _directoryUpdate();
        }

        const FileBrowserOptions& DirectoryWidget::getOptions() const
        {
            return _p->options;
        }

        void DirectoryWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            std::vector<int> columns;
            for (const auto& button : p.buttons)
            {
                if (columns.empty())
                {
                    columns = button->getTextWidths();
                }
                else
                {
                    const auto textWidths = button->getTextWidths();
                    for (size_t i = 0; i < columns.size() && i < textWidths.size(); ++i)
                    {
                        columns[i] = std::max(columns[i], textWidths[i]);
                    }
                }
            }
            if (!columns.empty())
            {
                for (size_t i = 0; i < columns.size() - 1; ++i)
                {
                    columns[i] += p.size.spacing;
                }
            }
            for (const auto& button : p.buttons)
            {
                button->setColumns(columns);
            }
            _p->layout->setGeometry(value);
        }

        void DirectoryWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.spacing = event.style->getSizeRole(SizeRole::Spacing, event.displayScale);

            std::vector<int> columns;
            for (const auto& button : p.buttons)
            {
                if (columns.empty())
                {
                    columns = button->getTextWidths();
                }
                else
                {
                    const auto textWidths = button->getTextWidths();
                    for (size_t i = 0; i < columns.size() && i < textWidths.size(); ++i)
                    {
                        columns[i] = std::max(columns[i], textWidths[i]);
                    }
                }
            }
            if (!columns.empty())
            {
                for (size_t i = 0; i < columns.size() - 1; ++i)
                {
                    columns[i] += p.size.spacing;
                }
            }
            for (const auto& button : p.buttons)
            {
                button->setColumns(columns);
            }

            _sizeHint = p.layout->getSizeHint();
            for (size_t i = 0; i < columns.size(); ++i)
            {
                _sizeHint.x += columns[i];
            }
        }

        void DirectoryWidget::_directoryUpdate()
        {
            TLRENDER_P();
            for (const auto& button : p.buttons)
            {
                button->setParent(nullptr);
            }
            p.buttons.clear();
            p.buttonToIndex.clear();
            p.buttonGroup->clearButtons();
            p.fileInfos = file::list(p.path, p.options.list);
            if (auto context = _context.lock())
            {
                for (size_t i = 0; i < p.fileInfos.size(); ++i)
                {
                    const file::FileInfo& fileInfo = p.fileInfos[i];
                    bool keep = true;
                    if (!p.options.filter.empty())
                    {
                        const std::string fileName = fileInfo.getPath().get(-1, false);
                        keep = string::contains(
                            fileName,
                            p.options.filter,
                            string::Compare::CaseInsensitive);
                    }
                    if (file::Type::File == fileInfo.getType() &&
                        !p.options.extension.empty())
                    {
                        keep = fileInfo.getPath().getExtension() == p.options.extension;
                    }
                    if (keep)
                    {
                        auto button = Button::create(fileInfo, context);
                        button->setParent(p.layout);
                        p.buttons.push_back(button);
                        p.buttonToIndex[button] = i;
                        p.buttonGroup->addButton(button);
                    }
                }
            }
        }
    }
}
