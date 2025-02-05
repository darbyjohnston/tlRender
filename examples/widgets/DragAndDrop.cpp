// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "DragAndDrop.h"

#include <tlUI/DrawUtil.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>

#include <tlTimeline/RenderUtil.h>

#include <sstream>

namespace tl
{
    namespace examples
    {
        namespace widgets
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
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(
                    "tl::examples::widgets::DragAndDropWidget",
                    context,
                    parent);
                DTK_P();

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
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<DragAndDropWidget>(new DragAndDropWidget);
                out->_init(number, context, parent);
                return out;
            }

            void DragAndDropWidget::setGeometry(const dtk::Box2I& value)
            {
                IWidget::setGeometry(value);
                _p->label->setGeometry(_geometry);
            }

            void DragAndDropWidget::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                DTK_P();
                p.border = event.style->getSizeRole(ui::SizeRole::Border, _displayScale);
                p.dragLength = event.style->getSizeRole(ui::SizeRole::DragLength, _displayScale);
                _sizeHint = p.label->getSizeHint();
            }

            void DragAndDropWidget::drawEvent(
                const dtk::Box2I& drawRect,
                const ui::DrawEvent& event)
            {
                IWidget::drawEvent(drawRect, event);
                DTK_P();
                const dtk::Box2I& g = _geometry;

                event.render->drawMesh(
                    ui::border(g, p.border),
                    event.style->getColorRole(ui::ColorRole::Border));

                const dtk::Box2I g2 = dtk::margin(g, -p.border);
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ui::ColorRole::Button));

                if (_mouse.press && dtk::contains(_geometry, _mouse.pos))
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
                DTK_P();
                if (_mouse.press)
                {
                    const float length = dtk::length(event.pos - _mouse.pressPos);
                    if (length > p.dragLength)
                    {
                        event.dndData = std::make_shared<DragAndDropData>(p.number);
                        const int w = _geometry.w();
                        const int h = _geometry.h();
                        event.dndCursor = dtk::Image::create(
                            w, h, dtk::ImageType::RGBA_U8);
                        uint8_t* p = event.dndCursor->getData();
                        for (int y = 0; y < h; ++y)
                        {
                            for (int x = 0; x < w; ++x)
                            {
                                p[0] = 255;
                                p[1] = 255;
                                p[2] = 255;
                                p[3] = 63;
                                p += 4;
                            }
                        }
                        event.dndCursorHotspot = _mouse.pos - _geometry.min;
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
                DTK_P();
                event.accept = true;
                p.dropTarget = true;
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::dragLeaveEvent(ui::DragAndDropEvent& event)
            {
                DTK_P();
                event.accept = true;
                p.dropTarget = false;
                _updates |= ui::Update::Draw;
            }

            void DragAndDropWidget::dropEvent(ui::DragAndDropEvent& event)
            {
                DTK_P();
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
                DTK_P();
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(3) << p.number;
                p.label->setText(ss.str());
            }

            struct DragAndDrop::Private
            {
                std::shared_ptr<ui::GridLayout> layout;
            };

            void DragAndDrop::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Drag and Drop",
                    "tl::examples::widgets::DragAndDrop",
                    context,
                    parent);
                DTK_P();

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
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<DragAndDrop>(new DragAndDrop);
                out->_init(context, parent);
                return out;
            }

            void DragAndDrop::setGeometry(const dtk::Box2I& value)
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
