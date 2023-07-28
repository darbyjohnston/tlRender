// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileEdit.h>

#include <tlUI/LineEdit.h>
#include <tlUI/FileBrowser.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>

#include <tlCore/File.h>

namespace tl
{
    namespace ui
    {
        struct FileEdit::Private
        {
            std::string path;
            std::shared_ptr<LineEdit> lineEdit;
            std::shared_ptr<PushButton> button;
            std::shared_ptr<HorizontalLayout> layout;
            std::function<void(const file::Path&)> fileCallback;
        };

        void FileEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FileEdit", context, parent);
            TLRENDER_P();

            setHStretch(Stretch::Expanding);

            p.path = file::getCWD();

            p.lineEdit = LineEdit::create(context);
            p.lineEdit->setHStretch(Stretch::Expanding);

            p.button = PushButton::create("Browse", context);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.lineEdit->setParent(p.layout);
            p.button->setParent(p.layout);

            p.button->setClickedCallback(
                [this]
                {
                    _openDialog();
                });
        }

        FileEdit::FileEdit() :
            _p(new Private)
        {}

        FileEdit::~FileEdit()
        {}

        std::shared_ptr<FileEdit> FileEdit::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileEdit>(new FileEdit);
            out->_init(context, parent);
            return out;
        }

        void FileEdit::setPath(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.path)
                return;
            p.path = value;
            p.lineEdit->setText(value);
        }

        const std::string& FileEdit::getFile() const
        {
            return _p->lineEdit->getText();
        }

        void FileEdit::setFileCallback(const std::function<void(const file::Path&)>& value)
        {
            _p->fileCallback = value;
        }

        void FileEdit::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void FileEdit::_openDialog()
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                if (auto eventLoop = getEventLoop().lock())
                {
                    if (auto fileBrowserSystem = context->getSystem<FileBrowserSystem>())
                    {
                        fileBrowserSystem->open(
                            eventLoop,
                            [this](const file::Path& value)
                            {
                                _p->path = value.getDirectory();
                                _p->lineEdit->setText(value.get());
                                if (_p->fileCallback)
                                {
                                    _p->fileCallback(value);
                                }
                            });
                    }
                }
            }
        }
    }
}
