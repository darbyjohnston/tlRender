// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "DragAndDrop.h"

#include <tlUI/DrawUtil.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <tlCore/Random.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            namespace
            {
                class MovableWidget;

                class DragAndDropData : public ui::DragAndDropData
                {
                public:
                    DragAndDropData(const std::shared_ptr<MovableWidget>&);

                    ~DragAndDropData() override;

                    const std::weak_ptr<MovableWidget>& getWidget() const;

                private:
                    std::weak_ptr<MovableWidget> _widget;
                };

                DragAndDropData::DragAndDropData(const std::shared_ptr<MovableWidget>& value) :
                    _widget(value)
                {}

                DragAndDropData::~DragAndDropData()
                {}

                const std::weak_ptr<MovableWidget>& DragAndDropData::getWidget() const
                {
                    return _widget;
                }

                class MovableWidget : public ui::IWidget
                {
                    TLRENDER_NON_COPYABLE(MovableWidget);

                protected:
                    void _init(
                        const std::string&,
                        const std::shared_ptr<system::Context>&,
                        const std::shared_ptr<IWidget>& parent = nullptr);

                    MovableWidget();

                public:
                    ~MovableWidget() override;

                    static std::shared_ptr<MovableWidget> create(
                        const std::string&,
                        const std::shared_ptr<system::Context>&,
                        const std::shared_ptr<IWidget>& parent = nullptr);

                    void setGeometry(const math::BBox2i&) override;
                    void sizeHintEvent(const ui::SizeHintEvent&) override;
                    void drawEvent(const math::BBox2i&, const ui::DrawEvent&) override;
                    void mouseEnterEvent() override;
                    void mouseLeaveEvent() override;
                    void mouseMoveEvent(ui::MouseMoveEvent&) override;
                    void mousePressEvent(ui::MouseClickEvent&) override;
                    void mouseReleaseEvent(ui::MouseClickEvent&) override;

                private:
                    std::shared_ptr<ui::Label> _label;
                    int _border = 0;
                    int _dragLength = 0;
                    bool _inside = false;
                    math::Vector2i _cursorPos;
                    bool _pressed = false;
                    math::Vector2i _pressedPos;
                };

                void MovableWidget::_init(
                    const std::string& text,
                    const std::shared_ptr<system::Context>& context,
                    const std::shared_ptr<IWidget>& parent)
                {
                    IWidget::_init("tl::examples::widgets_gl::MovableWidget", context, parent);
                    _label = ui::Label::create(context, shared_from_this());
                    _label->setText(text);
                    _label->setHAlign(ui::HAlign::Center);
                    _label->setVAlign(ui::VAlign::Center);
                }

                MovableWidget::MovableWidget()
                {}

                MovableWidget::~MovableWidget()
                {}

                std::shared_ptr<MovableWidget> MovableWidget::create(
                    const std::string& text,
                    const std::shared_ptr<system::Context>& context,
                    const std::shared_ptr<IWidget>& parent)
                {
                    auto out = std::shared_ptr<MovableWidget>(new MovableWidget);
                    out->_init(text, context, parent);
                    return out;
                }

                void MovableWidget::setGeometry(const math::BBox2i& value)
                {
                    IWidget::setGeometry(value);
                    _label->setGeometry(_geometry);
                }

                void MovableWidget::sizeHintEvent(const ui::SizeHintEvent& event)
                {
                    IWidget::sizeHintEvent(event);
                    _border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);
                    _sizeHint = _label->getSizeHint();
                }

                void MovableWidget::drawEvent(const math::BBox2i& drawRect, const ui::DrawEvent& event)
                {
                    IWidget::drawEvent(drawRect, event);

                    const math::BBox2i& g = _geometry;
                    event.render->drawMesh(
                        ui::border(g.margin(-_border), _border),
                        math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::Border));

                    const math::BBox2i g2 = g.margin(-_border);
                    event.render->drawRect(
                        g2,
                        event.style->getColorRole(ui::ColorRole::Button));

                    if (_pressed && _geometry.contains(_cursorPos))
                    {
                        event.render->drawRect(
                            g2,
                            event.style->getColorRole(ui::ColorRole::Pressed));
                    }
                    else if (_inside)
                    {
                        event.render->drawRect(
                            g2,
                            event.style->getColorRole(ui::ColorRole::Hover));
                    }
                }

                void MovableWidget::mouseEnterEvent()
                {
                    _inside = true;
                }

                void MovableWidget::mouseLeaveEvent()
                {
                    _inside = false;
                }

                void MovableWidget::mouseMoveEvent(ui::MouseMoveEvent& event)
                {
                    event.accept = true;
                    _cursorPos = event.pos;
                    if (_pressed)
                    {
                        event.accept = true;
                        const float length = math::length(event.pos - _pressedPos);
                        if (length > _dragLength)
                        {
                            event.dragAndDropData = std::make_shared<DragAndDropData>(
                                std::dynamic_pointer_cast<MovableWidget>(shared_from_this()));
                        }
                    }
                }

                void MovableWidget::mousePressEvent(ui::MouseClickEvent& event)
                {
                    event.accept = true;
                    _pressed = true;
                    _pressedPos = event.pos;
                }

                void MovableWidget::mouseReleaseEvent(ui::MouseClickEvent& event)
                {
                    event.accept = true;
                    _pressed = false;
                }

                class ContainerWidget : public ui::IWidget
                {
                    TLRENDER_NON_COPYABLE(ContainerWidget);

                protected:
                    void _init(
                        const std::shared_ptr<system::Context>&,
                        const std::shared_ptr<IWidget>& parent = nullptr);

                    ContainerWidget();

                public:
                    ~ContainerWidget() override;

                    static std::shared_ptr<ContainerWidget> create(
                        const std::shared_ptr<system::Context>&,
                        const std::shared_ptr<IWidget>& parent = nullptr);

                    void setGeometry(const math::BBox2i&) override;
                    void sizeHintEvent(const ui::SizeHintEvent&) override;
                    void drawOverlayEvent(const math::BBox2i&, const ui::DrawEvent&) override;
                    void dragEnterEvent(ui::DragAndDropEvent&) override;
                    void dragLeaveEvent(ui::DragAndDropEvent&) override;
                    void dragMoveEvent(ui::DragAndDropEvent&) override;
                    void dropEvent(ui::DragAndDropEvent&) override;

                private:
                    int _margin = 0;
                    int _spacing = 0;
                    bool _drag = false;
                    std::vector<math::BBox2i> _dropTargets;
                    int _dropTarget = -1;
                };

                void ContainerWidget::_init(
                    const std::shared_ptr<system::Context>& context,
                    const std::shared_ptr<IWidget>& parent)
                {
                    IWidget::_init("tl::examples::widgets_gl::ContainerWidget", context, parent);

                    setVStretch(ui::Stretch::Expanding);
                    setBackgroundRole(ui::ColorRole::Base);

                    MovableWidget::create("One", context, shared_from_this());
                    MovableWidget::create("Two", context, shared_from_this());
                    MovableWidget::create("Three", context, shared_from_this());
                }

                ContainerWidget::ContainerWidget()
                {}

                ContainerWidget::~ContainerWidget()
                {}

                std::shared_ptr<ContainerWidget> ContainerWidget::create(
                    const std::shared_ptr<system::Context>& context,
                    const std::shared_ptr<IWidget>& parent)
                {
                    auto out = std::shared_ptr<ContainerWidget>(new ContainerWidget);
                    out->_init(context, parent);
                    return out;
                }

                void ContainerWidget::setGeometry(const math::BBox2i& value)
                {
                    IWidget::setGeometry(value);
                    const math::BBox2i g = _geometry.margin(-_margin);

                    int w = g.w();
                    if (!_children.empty())
                    {
                        w -= _spacing * (_children.size() - 1);
                        w /= _children.size();
                    }
                    int x = g.min.x;
                    _dropTargets.clear();
                    for (const auto& child : _children)
                    {
                        const math::BBox2i bbox(
                            x,
                            g.min.y,
                            w,
                            g.h());
                        child->setGeometry(bbox);
                        _dropTargets.push_back(bbox);
                        x += w + _spacing;
                    }
                }

                void ContainerWidget::sizeHintEvent(const ui::SizeHintEvent& event)
                {
                    IWidget::sizeHintEvent(event);

                    _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall, event.displayScale);
                    _spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall, event.displayScale);

                    _sizeHint = math::Vector2i();
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.x += sizeHint.x;
                        _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
                    }
                    if (!_children.empty())
                    {
                        _sizeHint.x += _spacing * (_children.size() - 1);
                    }
                    _sizeHint.x += _margin * 2;
                    _sizeHint.y += _margin * 2;
                }

                void ContainerWidget::drawOverlayEvent(const math::BBox2i& drawRect, const ui::DrawEvent& event)
                {
                    IWidget::drawOverlayEvent(drawRect, event);
                    if (_drag)
                    {
                        if (_dropTarget >= 0 && _dropTarget < _dropTargets.size())
                        {
                            const math::BBox2i& bbox = _dropTargets[_dropTarget];
                            event.render->drawMesh(
                                ui::border(bbox.margin(_margin), _margin),
                                math::Vector2i(),
                                imaging::Color4f(1.F, .7F, 0.F, .5F));
                        }
                    }
                }

                void ContainerWidget::dragEnterEvent(ui::DragAndDropEvent& event)
                {
                    event.accept = true;
                    _drag = true;
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }

                void ContainerWidget::dragLeaveEvent(ui::DragAndDropEvent& event)
                {
                    event.accept = true;
                    _drag = false;
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }

                void ContainerWidget::dragMoveEvent(ui::DragAndDropEvent& event)
                {
                    event.accept = true;
                    int dropTarget = -1;
                    for (size_t i = 0; i < _dropTargets.size(); ++i)
                    {
                        if (_dropTargets[i].contains(event.pos))
                        {
                            dropTarget = i;
                            break;
                        }
                    }
                    if (dropTarget != _dropTarget)
                    {
                        _dropTarget = dropTarget;
                        _updates |= ui::Update::Size;
                        _updates |= ui::Update::Draw;
                    }
                }

                void ContainerWidget::dropEvent(ui::DragAndDropEvent& event)
                {
                    if (auto data = std::dynamic_pointer_cast<DragAndDropData>(event.data))
                    {
                        event.accept = true;
                        if (_dropTarget != -1)
                        {
                            if (auto widget = data->getWidget().lock())
                            {
                                widget->setParent(shared_from_this());
                                auto i = std::find(_children.begin(), _children.end(), widget);
                                _children.erase(i);
                                i = _children.begin();
                                for (int j = 0; j < _dropTarget && j < _children.size(); ++j)
                                {
                                    ++i;
                                }
                                _children.insert(i, widget);
                            }
                        }
                        _updates |= ui::Update::Size;
                        _updates |= ui::Update::Draw;
                    }
                }
            }

            struct DragAndDrop::Private
            {
                std::shared_ptr<ui::VerticalLayout> layout;
            };

            void DragAndDrop::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("DragAndDrop", context);
                TLRENDER_P();
                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                ContainerWidget::create(context, p.layout);
                ContainerWidget::create(context, p.layout);
                ContainerWidget::create(context, p.layout);
            }

            DragAndDrop::DragAndDrop() :
                _p(new Private)
            {}

            DragAndDrop::~DragAndDrop()
            {}

            std::shared_ptr<DragAndDrop> DragAndDrop::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<DragAndDrop>(new DragAndDrop);
                out->_init(context);
                return out;
            }

            void DragAndDrop::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
