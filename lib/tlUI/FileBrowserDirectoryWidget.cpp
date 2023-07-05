// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct DirectoryWidget::Private
        {
            std::string path;
            file::ListOptions listOptions;
            std::vector<file::FileInfo> fileInfos;
            std::vector<std::shared_ptr<Button> > buttons;
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

            p.buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);

            p.buttonGroup->setClickedCallback(
                [this](int value)
                {
                    TLRENDER_P();
                    if (value >= 0 &&
                        value < p.fileInfos.size())
                    {
                        const file::FileInfo& fileInfo = p.fileInfos[value];
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

        void DirectoryWidget::setPath(
            const std::string& value,
            const file::ListOptions& listOptions)
        {
            TLRENDER_P();
            if (value == p.path && listOptions == p.listOptions)
                return;
            p.path = value;
            p.listOptions = listOptions;
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
            p.buttonGroup->clearButtons();
            auto listOptions = p.listOptions;
            listOptions.sequence = false;
            p.fileInfos = file::list(p.path, listOptions);
            if (auto context = _context.lock())
            {
                for (auto fileInfo : p.fileInfos)
                {
                    auto button = Button::create(fileInfo, context);
                    button->setParent(p.layout);
                    p.buttons.push_back(button);
                    p.buttonGroup->addButton(button);
                }
            }
        }
    }
}
