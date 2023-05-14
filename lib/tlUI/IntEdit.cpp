// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEdit.h>

#include <tlUI/IntModel.h>
#include <tlUI/LineEdit.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct IntEdit::Private
        {
            std::shared_ptr<IntModel> model;
            std::shared_ptr<LineEdit> lineEdit;
            int digits = 3;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            std::shared_ptr<observer::ValueObserver<int> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::IntRange> > rangeObserver;
        };

        void IntEdit::_init(
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntEdit", context, parent);
            TLRENDER_P();

            p.lineEdit = LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(FontRole::Mono);

            p.model = model;
            if (!p.model)
            {
                p.model = IntModel::create(context);
            }

            p.lineEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->model->setValue(std::atoi(value.c_str()));
                    _textUpdate();
                });
            p.lineEdit->setFocusCallback(
                [this](bool value)
                {
                    if (!value)
                    {
                        _textUpdate();
                    }
                });

            p.valueObserver = observer::ValueObserver<int>::create(
                p.model->observeValue(),
                [this](int)
                {
                    _textUpdate();
                });

            p.rangeObserver = observer::ValueObserver<math::IntRange>::create(
                p.model->observeRange(),
                [this](const math::IntRange&)
                {
                    _textUpdate();
                });

            _textUpdate();
        }

        IntEdit::IntEdit() :
            _p(new Private)
        {}

        IntEdit::~IntEdit()
        {}

        std::shared_ptr<IntEdit> IntEdit::create(
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IntEdit>(new IntEdit);
            out->_init(model, context, parent);
            return out;
        }

        const std::shared_ptr<IntModel>& IntEdit::getModel() const
        {
            return _p->model;
        }

        void IntEdit::setDigits(int value)
        {
            TLRENDER_P();
            if (value == p.digits)
                return;
            p.digits = value;
            _textUpdate();
        }

        void IntEdit::setFontRole(FontRole value)
        {
            _p->lineEdit->setFontRole(value);
        }

        void IntEdit::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->lineEdit->setGeometry(value);
        }

        void IntEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->lineEdit->getSizeHint();
        }

        void IntEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (_enabled && p.model)
            {
                switch (event.key)
                {
                case Key::Down:
                    event.accept = true;
                    p.model->decrementStep();
                    break;
                case Key::Up:
                    event.accept = true;
                    p.model->incrementStep();
                    break;
                case Key::PageUp:
                    event.accept = true;
                    p.model->incrementLargeStep();
                    break;
                case Key::PageDown:
                    event.accept = true;
                    p.model->decrementLargeStep();
                    break;
                }
            }
        }

        void IntEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void IntEdit::_textUpdate()
        {
            TLRENDER_P();
            std::string text;
            std::string format;
            if (p.model)
            {
                text = string::Format("{0}").arg(p.model->getValue());
                format = string::Format("{0}").arg(0, p.digits);
            }
            p.lineEdit->setText(text);
            p.lineEdit->setFormat(format);
        }
    }
}
