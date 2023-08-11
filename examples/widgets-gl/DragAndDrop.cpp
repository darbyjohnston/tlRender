// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "DragAndDrop.h"

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>

#include <sstream>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            DragAndDropData::DragAndDropData(int value) :
                _number(value)
            {}

            DragAndDropData::~DragAndDropData()
            {}

            int DragAndDropData::getNumber() const
            {
                return _number;
            }

            struct DragAndDropWidget::Private
            {
                int number = 0;
                std::shared_ptr<ui::Label> label;
                int border = 0;
                int dragLength = 0;
                bool dropTarget = false;
            };

            void DragAndDropWidget::_init(
                int number,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(
                    "tl::examples::widgets_gl::DragAndDropWidget",
                    context,
                    parent);
                TLRENDER_P();

                _setMouseHover(true);
                _setMousePress(true);

                p.number = number;

                p.label = ui::Label::create(context, shared_from_this());
                p.label->setHAlign(ui::HAlign::Center);
                p.label->setVAlign(ui::VAlign::Center);
                p.label->setMarginRole(ui::SizeRole::Margin);

                _textUpdate();
            }

            DragAndDropWidget::DragAndDropWidget() :
                _p(new Private)
            {}

            DragAndDropWidget::~DragAndDropWidget()
            {}

            std::shared_ptr<DragAndDropWidget> DragAndDropWidget::create(
                int number,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<DragAndDropWidget>(new DragAndDropWidget);
                out->_init(number, context, parent);
                return out;
            }

            void DragAndDropWidget::setGeometry(const math::Box2i& value)
            {
                IWidget::setGeometry(value);
                _p->label->setGeometry(_geometry);
            }

            void DragAndDropWidget::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                TLRENDER_P();
                p.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);
                p.dragLength = event.style->getSizeRole(ui::SizeRole::DragLength, event.displayScale);
                _sizeHint = p.label->getSizeHint();
            }

            void DragAndDropWidget::drawEvent(const math::Box2i& drawRect, const ui::DrawEvent& event)
            {
                IWidget::drawEvent(drawRect, event);
                TLRENDER_P();

                const math::Box2i& g = _geometry;
                event.render->drawMesh(
                    ui::border(g, p.border),
                    math::Vector2i(),
                    event.style->getColorRole(ui::ColorRole::Border));

                const math::Box2i g2 = g.margin(-p.border);
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ui::ColorRole::Button));

                if (_mouse.press && _geometry.contains(_mouse.pos))
                {
                    event.render->drawRect(
                        g2,
                        event.style->getColorRole(ui::ColorRole::Pressed));
                }
                else if (_mouse.inside)
                {
                    event.render->drawRect(
                        g2,
                        event.style->getColorRole(ui::ColorRole::Hover));
                }

                if (p.dropTarget)
                {
                    auto color = event.style->getColorRole(ui::ColorRole::Checked);
                    color.a = .5F;
                    event.render->drawRect(g2, color);
                }
            }

            void DragAndDropWidget::mouseEnterEvent()
            {
                IWidget::mouseEnterEvent();
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::mouseLeaveEvent()
            {
                IWidget::mouseLeaveEvent();
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::mouseMoveEvent(ui::MouseMoveEvent& event)
            {
                IWidget::mouseMoveEvent(event);
                TLRENDER_P();
                if (_mouse.press)
                {
                    const float length = math::length(event.pos - _mouse.pressPos);
                    if (length > p.dragLength)
                    {
                        if (auto eventLoop = getEventLoop().lock())
                        {
                            event.dndData = std::make_shared<DragAndDropData>(p.number);
                            event.dndCursor = eventLoop->screenshot(shared_from_this());
                            event.dndCursorHotspot = _mouse.pos - _geometry.min;
                        }
                    }
                }
            }

            void DragAndDropWidget::mousePressEvent(ui::MouseClickEvent& event)
            {
                IWidget::mousePressEvent(event);
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::mouseReleaseEvent(ui::MouseClickEvent& event)
            {
                IWidget::mouseReleaseEvent(event);
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::dragEnterEvent(ui::DragAndDropEvent& event)
            {
                TLRENDER_P();
                event.accept = true;
                p.dropTarget = true;
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::dragLeaveEvent(ui::DragAndDropEvent& event)
            {
                TLRENDER_P();
                event.accept = true;
                p.dropTarget = false;
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::dropEvent(ui::DragAndDropEvent& event)
            {
                TLRENDER_P();
                if (auto data = std::dynamic_pointer_cast<DragAndDropData>(event.data))
                {
                    event.accept = true;
                    p.number = data->getNumber();
                    _textUpdate();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }
            }

            void DragAndDropWidget::_textUpdate()
            {
                TLRENDER_P();
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(3) << p.number;
                p.label->setText(ss.str());
            }

            struct DragAndDrop::Private
            {
                std::shared_ptr<ui::GridLayout> layout;
            };

            void DragAndDrop::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Drag and Drop",
                    "tl::examples::widgets_gl::DragAndDrop",
                    context,
                    parent);
                TLRENDER_P();

                p.layout = ui::GridLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);

                for (size_t i = 0; i < 10; ++i)
                {
                    for (size_t j = 0; j < 10; ++j)
                    {
                        auto widget = DragAndDropWidget::create(
                            i * 10 + j,
                            context,
                            p.layout);
                        p.layout->setGridPos(widget, i, j);
                    }
                }
            }

            DragAndDrop::DragAndDrop() :
                _p(new Private)
            {}

            DragAndDrop::~DragAndDrop()
            {}

            std::shared_ptr<DragAndDrop> DragAndDrop::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<DragAndDrop>(new DragAndDrop);
                out->_init(context, parent);
                return out;
            }

            void DragAndDrop::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void DragAndDrop::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        }
    }
}
