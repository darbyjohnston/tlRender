// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/DoubleEdit.h>

#include <tlUI/IncButtons.h>
#include <tlUI/LineEdit.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct DoubleEdit::Private
        {
            std::shared_ptr<DoubleModel> model;
            int digits = 3;
            int precision = 2;
            std::shared_ptr<LineEdit> lineEdit;
            std::shared_ptr<DoubleIncButtons> incButtons;
            std::shared_ptr<HorizontalLayout> layout;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            std::function<void(double)> callback;
            
            std::shared_ptr<observer::ValueObserver<double> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::DoubleRange> > rangeObserver;
        };

        void DoubleEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::DoubleEdit", context, parent);
            TLRENDER_P();

            p.model = model;
            if (!p.model)
            {
                p.model = DoubleModel::create(context);
            }

            p.lineEdit = LineEdit::create(context);
            p.lineEdit->setFontRole(FontRole::Mono);

            p.incButtons = DoubleIncButtons::create(p.model, context);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.lineEdit->setParent(p.layout);
            p.incButtons->setParent(p.layout);

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
                [this](double value)
                {
                    _textUpdate();
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });

            p.rangeObserver = observer::ValueObserver<math::DoubleRange>::create(
                p.model->observeRange(),
                [this](const math::DoubleRange& value)
                {
                    _p->digits = math::digits(value.getMax());
                    _updates |= Update::Size;
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
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DoubleEdit>(new DoubleEdit);
            out->_init(context, model, parent);
            return out;
        }

        double DoubleEdit::getValue() const
        {
            return _p->model->getValue();
        }

        void DoubleEdit::setValue(double value)
        {
            _p->model->setValue(value);
        }

        void DoubleEdit::setCallback(const std::function<void(double)>& value)
        {
            _p->callback = value;
        }

        const math::DoubleRange& DoubleEdit::getRange() const
        {
            return _p->model->getRange();
        }

        void DoubleEdit::setRange(const math::DoubleRange& value)
        {
            _p->model->setRange(value);
        }

        void DoubleEdit::setStep(double value)
        {
            _p->model->setStep(value);
        }

        void DoubleEdit::setLargeStep(double value)
        {
            _p->model->setLargeStep(value);
        }

        const std::shared_ptr<DoubleModel>& DoubleEdit::getModel() const
        {
            return _p->model;
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

        void DoubleEdit::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void DoubleEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void DoubleEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && p.model && 0 == event.modifiers)
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
