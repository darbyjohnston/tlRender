// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimeEdit.h>

#include <tlTimeline/TimeUnits.h>

#include <feather-tk/ui/LineEdit.h>
#include <feather-tk/ui/IncButtons.h>
#include <feather-tk/ui/RowLayout.h>

namespace tl
{
    namespace timelineui
    {
        struct TimeEdit::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            OTIO_NS::RationalTime value = time::invalidTime;
            std::function<void(const OTIO_NS::RationalTime&)> callback;
            std::shared_ptr<feather_tk::LineEdit> lineEdit;
            std::shared_ptr<feather_tk::IncButtons> incButtons;
            std::shared_ptr<feather_tk::HorizontalLayout> layout;

            std::shared_ptr<feather_tk::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeEdit::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::TimeEdit", parent);
            FEATHER_TK_P();

            p.timeUnitsModel = timeUnitsModel;
            if (!p.timeUnitsModel)
            {
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            }

            p.lineEdit = feather_tk::LineEdit::create(context, shared_from_this());
            p.lineEdit->setFontRole(feather_tk::FontRole::Mono);
            p.lineEdit->setHStretch(feather_tk::Stretch::Expanding);

            p.incButtons = feather_tk::IncButtons::create(context);

            p.layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(feather_tk::SizeRole::SpacingTool);
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

            p.timeUnitsObserver = feather_tk::ValueObserver<timeline::TimeUnits>::create(
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
            const std::shared_ptr<feather_tk::Context>& context,
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
            FEATHER_TK_P();
            if (value.strictly_equal(p.value))
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeEdit::setCallback(const std::function<void(const OTIO_NS::RationalTime&)>& value)
        {
            _p->callback = value;
        }

        void TimeEdit::selectAll()
        {
            _p->lineEdit->selectAll();
        }

        void TimeEdit::setFontRole(feather_tk::FontRole value)
        {
            _p->lineEdit->setFontRole(value);
        }

        void TimeEdit::setGeometry(const feather_tk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void TimeEdit::takeKeyFocus()
        {
            _p->lineEdit->takeKeyFocus();
        }

        void TimeEdit::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void TimeEdit::keyPressEvent(feather_tk::KeyEvent& event)
        {
            FEATHER_TK_P();
            if (isEnabled() && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case feather_tk::Key::Up:
                    event.accept = true;
                    _commitValue(
                        p.value +
                        OTIO_NS::RationalTime(1.0, p.value.rate()));
                    break;
                case feather_tk::Key::Down:
                    event.accept = true;
                    _commitValue(
                        p.value -
                        OTIO_NS::RationalTime(1.0, p.value.rate()));
                    break;
                case feather_tk::Key::PageUp:
                    event.accept = true;
                    _commitValue(
                        p.value +
                        OTIO_NS::RationalTime(p.value.rate(), p.value.rate()));
                    break;
                case feather_tk::Key::PageDown:
                    event.accept = true;
                    _commitValue(
                        p.value -
                        OTIO_NS::RationalTime(p.value.rate(), p.value.rate()));
                    break;
                default: break;
                }
            }
        }

        void TimeEdit::keyReleaseEvent(feather_tk::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimeEdit::_commitValue(const std::string& value)
        {
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            p.value = value;
            _textUpdate();
            if (p.callback)
            {
                p.callback(p.value);
            }
        }

        void TimeEdit::_textUpdate()
        {
            FEATHER_TK_P();
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
