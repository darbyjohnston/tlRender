// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimeEdit.h>

#include <tlUI/LineEdit.h>

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

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            std::shared_ptr<observer::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeEdit::_init(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimeEdit", context, parent);
            TLRENDER_P();

            p.lineEdit = LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(FontRole::Mono);

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

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

        void TimeEdit::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->lineEdit->setGeometry(value);
        }

        void TimeEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->lineEdit->getSizeHint();
        }

        void TimeEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled())
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
                switch (p.timeUnitsModel->getTimeUnits())
                {
                case timeline::TimeUnits::Frames:
                    tmp = otime::RationalTime::from_frames(
                        atoi(value.c_str()),
                        p.value.rate());
                    break;
                case timeline::TimeUnits::Seconds:
                    tmp = otime::RationalTime::from_seconds(
                        atof(value.c_str()),
                        p.value.rate());
                    break;
                case timeline::TimeUnits::Timecode:
                    tmp = otime::RationalTime::from_timecode(
                        value,
                        p.value.rate(),
                        &errorStatus);
                    break;
                default: break;
                }
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
                switch (p.timeUnitsModel->getTimeUnits())
                {
                case timeline::TimeUnits::Frames:
                    text = string::Format("{0}").arg(p.value.to_frames());
                    format = "000000";
                    break;
                case timeline::TimeUnits::Seconds:
                    text = string::Format("{0}").arg(p.value.to_seconds(), 2);
                    format = "000000.00";
                    break;
                case timeline::TimeUnits::Timecode:
                    text = p.value.to_timecode();
                    format = "00:00:00;00";
                    break;
                default: break;
                }
            }
            p.lineEdit->setText(text);
            p.lineEdit->setFormat(format);
        }
    }
}
