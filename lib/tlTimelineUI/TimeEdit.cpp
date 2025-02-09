// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimeEdit.h>

#include <tlTimeline/TimeUnits.h>

#include <dtk/ui/LineEdit.h>
#include <dtk/ui/IncButtons.h>
#include <dtk/ui/RowLayout.h>

namespace tl
{
    namespace timelineui
    {
        struct TimeEdit::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            OTIO_NS::RationalTime value = time::invalidTime;
            std::function<void(const OTIO_NS::RationalTime&)> callback;
            std::shared_ptr<dtk::LineEdit> lineEdit;
            std::shared_ptr<dtk::IncButtons> incButtons;
            std::shared_ptr<dtk::HorizontalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeEdit::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::TimeEdit", parent);
            DTK_P();

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

            p.lineEdit = dtk::LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(dtk::FontRole::Mono);
            p.lineEdit->setHStretch(dtk::Stretch::Expanding);

            p.incButtons = dtk::IncButtons::create(context);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::SpacingTool);
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
                    _commitValue(_p->value + OTIO_NS::RationalTime(1.0, _p->value.rate()));
                });
            p.incButtons->setDecCallback(
                [this]
                {
                    _commitValue(_p->value + OTIO_NS::RationalTime(-1.0, _p->value.rate()));
                });

            p.timeUnitsObserver = dtk::ValueObserver<timeline::TimeUnits>::create(
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeEdit>(new TimeEdit);
            out->_init(context, timeUnitsModel, parent);
            return out;
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& TimeEdit::getTimeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        const OTIO_NS::RationalTime& TimeEdit::getValue() const
        {
            return _p->value;
        }

        void TimeEdit::setValue(const OTIO_NS::RationalTime& value)
        {
            DTK_P();
            if (value.strictly_equal(p.value))
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeEdit::setCallback(const std::function<void(const OTIO_NS::RationalTime&)>& value)
        {
            _p->callback = value;
        }

        void TimeEdit::setFontRole(dtk::FontRole value)
        {
            _p->lineEdit->setFontRole(value);
        }

        void TimeEdit::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void TimeEdit::takeKeyFocus()
        {
            _p->lineEdit->takeKeyFocus();
        }

        void TimeEdit::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void TimeEdit::keyPressEvent(dtk::KeyEvent& event)
        {
            DTK_P();
            if (isEnabled() && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case dtk::Key::Up:
                    event.accept = true;
                    _commitValue(
                        p.value +
                        OTIO_NS::RationalTime(1.0, p.value.rate()));
                    break;
                case dtk::Key::Down:
                    event.accept = true;
                    _commitValue(
                        p.value -
                        OTIO_NS::RationalTime(1.0, p.value.rate()));
                    break;
                case dtk::Key::PageUp:
                    event.accept = true;
                    _commitValue(
                        p.value +
                        OTIO_NS::RationalTime(p.value.rate(), p.value.rate()));
                    break;
                case dtk::Key::PageDown:
                    event.accept = true;
                    _commitValue(
                        p.value -
                        OTIO_NS::RationalTime(p.value.rate(), p.value.rate()));
                    break;
                default: break;
                }
            }
        }

        void TimeEdit::keyReleaseEvent(dtk::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimeEdit::_commitValue(const std::string& value)
        {
            DTK_P();
            OTIO_NS::RationalTime tmp = time::invalidTime;
            opentime::ErrorStatus errorStatus;
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
                !opentime::is_error(errorStatus);
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

        void TimeEdit::_commitValue(const OTIO_NS::RationalTime& value)
        {
            DTK_P();
            p.value = value;
            _textUpdate();
            if (p.callback)
            {
                p.callback(p.value);
            }
        }

        void TimeEdit::_textUpdate()
        {
            DTK_P();
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
