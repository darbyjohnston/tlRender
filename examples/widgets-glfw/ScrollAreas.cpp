// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ScrollAreas.h"

#include <tlUI/GridLayout.h>
#include <tlUI/LayoutUtil.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            namespace
            {
                class ScrollAreasWidget : public ui::IWidget
                {
                protected:
                    void _init(
                        const math::Vector2i& cellCount,
                        const std::shared_ptr<system::Context>&);

                    ScrollAreasWidget();

                public:
                    ~ScrollAreasWidget();

                    static std::shared_ptr<ScrollAreasWidget> create(
                        const math::Vector2i& cellCount,
                        const std::shared_ptr<system::Context>&);

                    void sizeHintEvent(const ui::SizeHintEvent&) override;
                    void clipEvent(
                        const math::BBox2i&,
                        bool,
                        const ui::ClipEvent&) override;
                    void drawEvent(
                        const math::BBox2i&,
                        const ui::DrawEvent&) override;

                private:
                    math::Vector2i _cellCount;
                    int _cellSize = 0;
                    int _margin = 0;
                    std::vector<math::Vector2i> _textSize;
                    std::vector<std::vector<std::shared_ptr<imaging::Glyph> > > _glyphs;
                };

                void ScrollAreasWidget::_init(
                    const math::Vector2i& cellCount,
                    const std::shared_ptr<system::Context>& context)
                {
                    IWidget::_init("ScrollAreasWidget", context);
                    _cellCount = cellCount;
                    _textSize.resize(_cellCount.x * _cellCount.y);
                    _glyphs.resize(_cellCount.x * _cellCount.y);
                }

                ScrollAreasWidget::ScrollAreasWidget()
                {}

                ScrollAreasWidget::~ScrollAreasWidget()
                {}

                std::shared_ptr<ScrollAreasWidget> ScrollAreasWidget::create(
                    const math::Vector2i& cellCount,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<ScrollAreasWidget>(new ScrollAreasWidget);
                    out->_init(cellCount, context);
                    return out;
                }

                void ScrollAreasWidget::sizeHintEvent(const ui::SizeHintEvent& event)
                {
                    IWidget::sizeHintEvent(event);
                    _margin = event.style->getSizeRole(ui::SizeRole::MarginLarge, event.displayScale);
                    const std::string format = string::Format("{0}, {1}").
                        arg(ui::format(_cellCount.x)).
                        arg(ui::format(_cellCount.y));
                    const auto fontInfo = event.style->getFontRole(ui::FontRole::Label, event.displayScale);
                    const math::Vector2i textSize = event.fontSystem->getSize(format, fontInfo);
                    _cellSize = textSize.x + _margin * 2;
                    _sizeHint.x = _cellCount.x * _cellSize;
                    _sizeHint.y = _cellCount.y * _cellSize;
                }

                void ScrollAreasWidget::clipEvent(
                    const math::BBox2i& clipRect,
                    bool clipped,
                    const ui::ClipEvent& event)
                {
                    IWidget::clipEvent(clipRect, clipped, event);
                    if (clipped)
                    {
                        for (auto& i : _glyphs)
                        {
                            i.clear();
                        }
                    }
                }

                void ScrollAreasWidget::drawEvent(
                    const math::BBox2i& drawRect,
                    const ui::DrawEvent& event)
                {
                    IWidget::drawEvent(drawRect, event);

                    const math::BBox2i& g = _geometry;

                    const auto fontInfo = event.style->getFontRole(ui::FontRole::Label, event.displayScale);
                    const auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
                    for (int y = 0; y < _cellCount.y; ++y)
                    {
                        for (int x = 0; x < _cellCount.x; ++x)
                        {
                            const bool even = ((x + y) % 2 == 0);

                            const math::BBox2i g2(
                                g.x() + x * _cellSize,
                                g.y() + y * _cellSize,
                                _cellSize,
                                _cellSize);
                            event.render->drawRect(
                                g2,
                                event.style->getColorRole(
                                    even ?
                                    ui::ColorRole::Window :
                                    ui::ColorRole::Button));

                            const size_t i = y * _cellCount.x + x;
                            if (_glyphs[i].empty())
                            {
                                const std::string text = string::Format("{0}, {1}").
                                    arg(y).
                                    arg(x);
                                _textSize[i] = event.fontSystem->getSize(text, fontInfo);
                                _glyphs[i] = event.fontSystem->getGlyphs(text, fontInfo);
                            }
                            event.render->drawText(
                                _glyphs[i],
                                g2.getCenter() - _textSize[i] / 2 + math::Vector2i(0, fontMetrics.ascender),
                                event.style->getColorRole(ui::ColorRole::Text));
                        }
                    }
                }
            }

            struct ScrollAreas::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void ScrollAreas::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("ScrollAreas", context);
                TLRENDER_P();

                auto widget0 = ScrollAreasWidget::create(math::Vector2i(10, 1), context);
                auto scrollWidget0 = ui::ScrollWidget::create(context, ui::ScrollType::Horizontal);
                scrollWidget0->setWidget(widget0);

                auto widget1 = ScrollAreasWidget::create(math::Vector2i(1, 10), context);
                auto scrollWidget1 = ui::ScrollWidget::create(context, ui::ScrollType::Vertical);
                scrollWidget1->setWidget(widget1);

                auto widget2 = ScrollAreasWidget::create(math::Vector2i(10, 10), context);
                auto scrollWidget2 = ui::ScrollWidget::create(context, ui::ScrollType::Both);
                scrollWidget2->setWidget(widget2);
                scrollWidget2->setHStretch(ui::Stretch::Expanding);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                scrollWidget0->setParent(p.layout);
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setVStretch(ui::Stretch::Expanding);
                scrollWidget1->setParent(hLayout);
                scrollWidget2->setParent(hLayout);
            }

            ScrollAreas::ScrollAreas() :
                _p(new Private)
            {}

            ScrollAreas::~ScrollAreas()
            {}

            std::shared_ptr<ScrollAreas> ScrollAreas::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<ScrollAreas>(new ScrollAreas);
                out->_init(context);
                return out;
            }

            void ScrollAreas::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
