// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimeEdit.h>

#include <tlUI/TimeUnitsModel.h>
#include <tlUI/LineEdit.h>

namespace tl
{
    namespace ui
    {
        struct TimeEdit::Private
        {
            otime::RationalTime time;
            std::function<void(const otime::RationalTime&)> timeCallback;
            std::shared_ptr<TimeUnitsModel> timeUnitsModel;
            std::shared_ptr<LineEdit> lineEdit;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            std::shared_ptr<observer::ValueObserver<TimeUnits> > timeUnitsObserver;
        };

        void TimeEdit::_init(
            const std::shared_ptr<TimeUnitsModel>& timeUnitsModel,
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
                p.timeUnitsModel = TimeUnitsModel::create(context);
            }

            p.lineEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->time = otime::RationalTime::from_timecode(
                        value,
                        _p->time.rate());
                    _textUpdate();
                    if (_p->timeCallback)
                    {
                        _p->timeCallback(_p->time);
                    }
                });
            p.lineEdit->setFocusCallback(
                [this](bool value)
                {
                    if (!value)
                    {
                        _textUpdate();
                    }
                });

            p.timeUnitsObserver = observer::ValueObserver<TimeUnits>::create(
                p.timeUnitsModel->observeTimeUnits(),
                [this](TimeUnits)
                {
                    _textUpdate();
                });

            _textUpdate();
        }

        TimeEdit::TimeEdit() :
            _p(new Private)
        {}

        TimeEdit::~TimeEdit()
        {}

        std::shared_ptr<TimeEdit> TimeEdit::create(
            const std::shared_ptr<TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeEdit>(new TimeEdit);
            out->_init(timeUnitsModel, context, parent);
            return out;
        }

        const std::shared_ptr<TimeUnitsModel>& TimeEdit::getTimeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        const otime::RationalTime& TimeEdit::getTime() const
        {
            return _p->time;
        }

        void TimeEdit::setTime(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (value == p.time)
                return;
            p.time = value;
            _textUpdate();
        }

        void TimeEdit::setTimeCallback(const std::function<void(const otime::RationalTime&)>& value)
        {
            _p->timeCallback = value;
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
            if (_enabled)
            {
                switch (event.key)
                {
                case Key::Down:
                    event.accept = true;
                    break;
                case Key::Up:
                    event.accept = true;
                    break;
                case Key::PageUp:
                    event.accept = true;
                    break;
                case Key::PageDown:
                    event.accept = true;
                    break;
                }
            }
        }

        void TimeEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
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
                case TimeUnits::Frames:
                    text = p.time.to_frames();
                    format = std::string(text.size(), '0');
                    break;
                case TimeUnits::Seconds:
                    text = p.time.to_seconds();
                    format = std::string(text.size(), '0');
                    break;
                case TimeUnits::Timecode:
                    text = p.time.to_timecode();
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
