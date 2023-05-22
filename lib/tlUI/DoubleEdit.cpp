// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/DoubleEdit.h>

#include <tlUI/DoubleModel.h>
#include <tlUI/LayoutUtil.h>
#include <tlUI/LineEdit.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct DoubleEdit::Private
        {
            std::shared_ptr<DoubleModel> model;
            std::shared_ptr<LineEdit> lineEdit;
            int digits = 3;
            int precision = 2;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;
            
            std::shared_ptr<observer::ValueObserver<double> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::DoubleRange> > rangeObserver;
        };

        void DoubleEdit::_init(
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::DoubleEdit", context, parent);
            TLRENDER_P();

            p.lineEdit = LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(FontRole::Mono);

            p.model = model;
            if (!p.model)
            {
                p.model = DoubleModel::create(context);
            }

            p.lineEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->model->setValue(std::atof(value.c_str()));
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

            p.valueObserver = observer::ValueObserver<double>::create(
                p.model->observeValue(),
                [this](double)
                {
                    _textUpdate();
                });

            p.rangeObserver = observer::ValueObserver<math::DoubleRange>::create(
                p.model->observeRange(),
                [this](const math::DoubleRange&)
                {
                    _textUpdate();
                });

            _textUpdate();
        }

        DoubleEdit::DoubleEdit() :
            _p(new Private)
        {}

        DoubleEdit::~DoubleEdit()
        {}

        std::shared_ptr<DoubleEdit> DoubleEdit::create(
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DoubleEdit>(new DoubleEdit);
            out->_init(model, context, parent);
            return out;
        }

        const std::shared_ptr<DoubleModel>& DoubleEdit::getModel() const
        {
            return _p->model;
        }

        void DoubleEdit::setDigits(int value)
        {
            TLRENDER_P();
            if (value == p.digits)
                return;
            p.digits = value;
            _textUpdate();
        }

        void DoubleEdit::setPrecision(int value)
        {
            TLRENDER_P();
            if (value == p.precision)
                return;
            p.precision = value;
            _textUpdate();
        }

        void DoubleEdit::setFontRole(FontRole value)
        {
            _p->lineEdit->setFontRole(value);
        }

        void DoubleEdit::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->lineEdit->setGeometry(value);
        }

        void DoubleEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->lineEdit->getSizeHint();
        }

        void DoubleEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && p.model)
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
                default: break;
                }
            }
        }

        void DoubleEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void DoubleEdit::_textUpdate()
        {
            TLRENDER_P();
            std::string text;
            std::string format;
            if (p.model)
            {
                text = string::Format("{0}").arg(p.model->getValue(), p.precision);
                format = string::Format("{0}").arg(0, p.digits + 1 + p.precision);
            }
            p.lineEdit->setText(text);
            p.lineEdit->setFormat(format);
        }
    }
}
