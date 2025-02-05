// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/FloatSlider.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct FloatSlider::Private
        {
            std::shared_ptr<FloatModel> model;

            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
                int border = 0;
                int handle = 0;
                dtk::FontMetrics fontMetrics;
            };
            SizeData size;

            std::function<void(float)> callback;

            std::shared_ptr<dtk::ValueObserver<float> > valueObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::RangeF> > rangeObserver;
        };

        void FloatSlider::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FloatSlider", context, parent);
            DTK_P();

            setAcceptsKeyFocus(true);
            setHStretch(Stretch::Expanding);
            _setMouseHover(true);
            _setMousePress(true);

            p.model = model;
            if (!p.model)
            {
                p.model = FloatModel::create(context);
            }

            p.valueObserver = dtk::ValueObserver<float>::create(
                p.model->observeValue(),
                [this](float value)
                {
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });

            p.rangeObserver = dtk::ValueObserver<dtk::RangeF>::create(
                p.model->observeRange(),
                [this](const dtk::RangeF&)
                {
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                });
        }

        FloatSlider::FloatSlider() :
            _p(new Private)
        {}

        FloatSlider::~FloatSlider()
        {}

        std::shared_ptr<FloatSlider> FloatSlider::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FloatSlider>(new FloatSlider);
            out->_init(context, model, parent);
            return out;
        }

        float FloatSlider::getValue() const
        {
            return _p->model->getValue();
        }

        void FloatSlider::setValue(float value)
        {
            _p->model->setValue(value);
        }

        void FloatSlider::setCallback(const std::function<void(float)>& value)
        {
            _p->callback = value;
        }

        const dtk::RangeF& FloatSlider::getRange() const
        {
            return _p->model->getRange();
        }

        void FloatSlider::setRange(const dtk::RangeF& value)
        {
            _p->model->setRange(value);
        }

        void FloatSlider::setStep(float value)
        {
            _p->model->setStep(value);
        }

        void FloatSlider::setLargeStep(float value)
        {
            _p->model->setLargeStep(value);
        }

        void FloatSlider::setDefaultValue(float value)
        {
            _p->model->setDefaultValue(value);
        }

        const std::shared_ptr<FloatModel>& FloatSlider::getModel() const
        {
            return _p->model;
        }

        void FloatSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.size = event.style->getSizeRole(SizeRole::Slider, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
                p.size.handle = event.style->getSizeRole(SizeRole::Handle, _displayScale);
                auto fontInfo = event.style->getFontRole(FontRole::Label, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);
            }
            p.size.sizeInit = false;

            _sizeHint.w =
                p.size.size +
                p.size.border * 6;
            _sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.border * 6;
        }

        void FloatSlider::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;

            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }
            else
            {
                event.render->drawMesh(
                    border(dtk::margin(g, -p.size.border), p.size.border),
                    event.style->getColorRole(ColorRole::Border));
            }

            event.render->drawRect(
                dtk::margin(g, -p.size.border * 2),
                event.style->getColorRole(ColorRole::Base));

            const dtk::Box2I g2 = _getSliderGeometry();
            //event.render->drawRect(
            //    g2,
            //    dtk::Color4F(1.F, 0.F, 0.F, .5F));
            int pos = 0;
            if (p.model)
            {
                pos = _valueToPos(p.model->getValue());
            }
            const dtk::Box2I g3(
                pos - p.size.handle / 2,
                g2.y(),
                p.size.handle,
                g2.h());
            event.render->drawRect(
                g3,
                event.style->getColorRole(ColorRole::Button));
            if (_mouse.press)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Hover));
            }
        }

        void FloatSlider::mouseEnterEvent()
        {
            IWidget::mouseEnterEvent();
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseMoveEvent(MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            DTK_P();
            if (_mouse.press && p.model)
            {
                p.model->setValue(_posToValue(_mouse.pos.x));
            }
        }

        void FloatSlider::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            if (p.model)
            {
                p.model->setValue(_posToValue(_mouse.pos.x));
            }
            takeKeyFocus();
            _updates |= Update::Draw;
        }

        void FloatSlider::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            _updates |= Update::Draw;
        }

        void FloatSlider::keyPressEvent(KeyEvent& event)
        {
            DTK_P();
            if (isEnabled() && p.model && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::Left:
                case Key::Down:
                    event.accept = true;
                    p.model->decrementStep();
                    break;
                case Key::Right:
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
                case Key::End:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().min());
                    break;
                case Key::Home:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().max());
                    break;
                case Key::Escape:
                    if (hasKeyFocus())
                    {
                        event.accept = true;
                        releaseKeyFocus();
                    }
                    break;
                default: break;
                }
            }
        }

        void FloatSlider::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        dtk::Box2I FloatSlider::_getSliderGeometry() const
        {
            DTK_P();
            return dtk::margin(_geometry,
                -(p.size.border * 3 + p.size.handle / 2),
                -(p.size.border * 3),
                -(p.size.border * 3 + p.size.handle / 2),
                -(p.size.border * 3));
        }

        float FloatSlider::_posToValue(int pos) const
        {
            DTK_P();
            const dtk::Box2I g = _getSliderGeometry();
            const float v = (pos - g.x()) / static_cast<float>(g.w());
            float out = 0.F;
            if (p.model)
            {
                const dtk::RangeF& range = p.model->getRange();
                out = range.min() + (range.max() - range.min()) * v;
            }
            return out;
        }

        int FloatSlider::_valueToPos(float value) const
        {
            DTK_P();
            const dtk::Box2I g = _getSliderGeometry();
            float v = 0.F;
            if (p.model)
            {
                const dtk::RangeF& range = p.model->getRange();
                if (range.min() != range.max())
                {
                    v = (value - range.min()) /
                        static_cast<float>(range.max() - range.min());
                }
            }
            return g.x() + g.w() * v;
        }
    }
}
