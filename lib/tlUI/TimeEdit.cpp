// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/TimeEdit.h>

#include <tlUI/LineEdit.h>
#include <tlUI/IncButtons.h>
#include <tlUI/RowLayout.h>

#include <tlTimeline/TimeUnits.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct TimeEdit::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            otime::RationalTime value = time::invalidTime;
            std::function<void(const otime::RationalTime&)> callback;
            std::shared_ptr<LineEdit> lineEdit;
            std::shared_ptr<IncButtons> incButtons;
            std::shared_ptr<HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeEdit::_init(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimeEdit", context, parent);
            TLRENDER_P();

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

            p.lineEdit = LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(FontRole::Mono);
            p.lineEdit->setHStretch(Stretch::Expanding);

            p.incButtons = IncButtons::create(context);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.lineEdit->setParent(p.layout);
            p.incButtons->setParent(p.layout);

            _textUpdate();

            p.lineEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _commitValue(value);
                });
            p.lineEdit->setFocusCallback(
                [this](bool value)
                {
                    if (!value)
                    {
                        _textUpdate();
                    }
                });

            p.incButtons->setIncCallback(
                [this]
                {
                    _commitValue(_p->value + otime::RationalTime(1.0, _p->value.rate()));
                });
            p.incButtons->setDecCallback(
                [this]
                {
                    _commitValue(_p->value + otime::RationalTime(-1.0, _p->value.rate()));
                });

            p.timeUnitsObserver = observer::ValueObserver<timeline::TimeUnits>::create(
                p.timeUnitsModel->observeTimeUnits(),
                [this](timeline::TimeUnits)
                {
                    _textUpdate();
                });
        }

        TimeEdit::TimeEdit() :
            _p(new Private)
        {}

        TimeEdit::~TimeEdit()
        {}

        std::shared_ptr<TimeEdit> TimeEdit::create(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeEdit>(new TimeEdit);
            out->_init(timeUnitsModel, context, parent);
            return out;
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& TimeEdit::getTimeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        const otime::RationalTime& TimeEdit::getValue() const
        {
            return _p->value;
        }

        void TimeEdit::setValue(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (time::compareExact(value, p.value))
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeEdit::setCallback(const std::function<void(const otime::RationalTime&)>& value)
        {
            _p->callback = value;
        }

        void TimeEdit::setFontRole(FontRole value)
        {
            _p->lineEdit->setFontRole(value);
        }

        void TimeEdit::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void TimeEdit::takeKeyFocus()
        {
            _p->lineEdit->takeKeyFocus();
        }

        void TimeEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void TimeEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::Up:
                    event.accept = true;
                    _commitValue(
                        p.value +
                        otime::RationalTime(1.0, p.value.rate()));
                    break;
                case Key::Down:
                    event.accept = true;
                    _commitValue(
                        p.value -
                        otime::RationalTime(1.0, p.value.rate()));
                    break;
                case Key::PageUp:
                    event.accept = true;
                    _commitValue(
                        p.value +
                        otime::RationalTime(p.value.rate(), p.value.rate()));
                    break;
                case Key::PageDown:
                    event.accept = true;
                    _commitValue(
                        p.value -
                        otime::RationalTime(p.value.rate(), p.value.rate()));
                    break;
                default: break;
                }
            }
        }

        void TimeEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void TimeEdit::_commitValue(const std::string& value)
        {
            TLRENDER_P();
            otime::RationalTime tmp = time::invalidTime;
            otime::ErrorStatus errorStatus;
            if (p.timeUnitsModel)
            {
                const timeline::TimeUnits timeUnits = p.timeUnitsModel->getTimeUnits();
                tmp = timeline::textToTime(
                    value,
                    p.value.rate(),
                    timeUnits,
                    &errorStatus);
            }
            const bool valid =
                tmp != time::invalidTime &&
                !otime::is_error(errorStatus);
            if (valid)
            {
                p.value = tmp;
            }
            _textUpdate();
            if (valid && p.callback)
            {
                p.callback(_p->value);
            }
        }

        void TimeEdit::_commitValue(const otime::RationalTime& value)
        {
            TLRENDER_P();
            p.value = value;
            _textUpdate();
            if (p.callback)
            {
                p.callback(p.value);
            }
        }

        void TimeEdit::_textUpdate()
        {
            TLRENDER_P();
            std::string text;
            std::string format;
            if (p.timeUnitsModel)
            {
                const timeline::TimeUnits timeUnits = p.timeUnitsModel->getTimeUnits();
                text = timeline::timeToText(p.value, timeUnits);
                format = timeline::formatString(timeUnits);
            }
            p.lineEdit->setText(text);
            p.lineEdit->setFormat(format);
        }
    }
}
