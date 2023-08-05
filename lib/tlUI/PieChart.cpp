// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/PieChart.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/LayoutUtil.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        PieChartData::PieChartData()
        {}

        PieChartData::PieChartData(
            const std::string& text,
            int percentage,
            const image::Color4f& color) :
            text(text),
            percentage(percentage),
            color(color)
        {}

        bool PieChartData::operator == (const PieChartData& other) const
        {
            return
                text == other.text &&
                percentage == other.percentage &&
                color == other.color;
        }

        bool PieChartData::operator != (const PieChartData& other) const
        {
            return !(*this == other);
        }

        struct PieChart::Private
        {
            std::vector<PieChartData> data;
            FontRole fontRole = FontRole::Label;
            int sizeMult = 5;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                image::FontMetrics fontMetrics;
                int pieDiameter = 0;
                math::Vector2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                struct PercentageLabel
                {
                    std::string text;
                    math::Vector2i size;
                    math::Vector2i pos;
                    std::vector<std::shared_ptr<image::Glyph> > glyphs;
                };
                std::vector<PercentageLabel> percentageLabels;
                std::vector<geom::TriangleMesh2> pieSliceMeshes;
                struct TextLabel
                {
                    std::string text;
                    math::Vector2i size;
                    math::Vector2i pos;
                    std::vector<std::shared_ptr<image::Glyph> > glyphs;
                    image::Color4f color;
                    geom::TriangleMesh2 circleMesh;
                };
                std::vector<TextLabel> textLabels;
            };
            DrawData draw;
        };

        void PieChart::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::PieChart", context, parent);
        }

        PieChart::PieChart() :
            _p(new Private)
        {}

        PieChart::~PieChart()
        {}

        std::shared_ptr<PieChart> PieChart::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PieChart>(new PieChart);
            out->_init(context, parent);
            return out;
        }

        void PieChart::setData(const std::vector<PieChartData>& value)
        {
            TLRENDER_P();
            if (value == p.data)
                return;
            p.data = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void PieChart::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void PieChart::setSizeMult(int value)
        {
            TLRENDER_P();
            if (value == p.sizeMult)
                return;
            p.sizeMult = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void PieChart::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.fontMetrics = event.getFontMetrics(p.fontRole);

            // Create the percentage labels.
            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.draw.percentageLabels.clear();
            int percentageWidthMax = 0;
            for (const auto& data : p.data)
            {
                Private::DrawData::PercentageLabel label;
                label.text = string::Format("{0}%").arg(data.percentage);
                label.size = event.fontSystem->getSize(label.text, fontInfo);
                label.glyphs = event.fontSystem->getGlyphs(label.text, fontInfo);
                p.draw.percentageLabels.push_back(label);
                percentageWidthMax = std::max(percentageWidthMax, label.size.x);
            }
            int a = 0;
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                auto& label = p.draw.percentageLabels[i];
                const int d = a + p.data[i].percentage / 2;
                const float r = p.size.fontMetrics.lineHeight * p.sizeMult / 2.F +
                    p.size.spacing +
                    label.size.x / 2.F;
                label.pos.x = cos(math::deg2rad(d / 100.F * 360.F - 90.F)) * r -
                    label.size.x / 2;
                label.pos.y = sin(math::deg2rad(d / 100.F * 360.F - 90.F)) * r -
                    label.size.y / 2;
                a += p.data[i].percentage;
            }

            // Create the pie slices.
            p.draw.pieSliceMeshes.clear();
            p.draw.pieSliceMeshes.resize(p.data.size());
            const float r = p.size.fontMetrics.lineHeight * p.sizeMult / 2.F;
            a = 0;
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                geom::TriangleMesh2& mesh = p.draw.pieSliceMeshes[i];
                const int d = p.data[i].percentage;
                const int inc = 2;
                for (int i = a; i < a + d; i += inc)
                {
                    const size_t size = mesh.v.size();
                    mesh.v.push_back(math::Vector2f(0.F, 0.F));
                    mesh.v.push_back(math::Vector2f(
                        cos(math::deg2rad(i / 100.F * 360.F - 90.F)) * r,
                        sin(math::deg2rad(i / 100.F * 360.F - 90.F)) * r));
                    mesh.v.push_back(math::Vector2f(
                        cos(math::deg2rad(std::min(i + inc, a + d) / 100.F * 360.F - 90.F)) * r,
                        sin(math::deg2rad(std::min(i + inc, a + d) / 100.F * 360.F - 90.F)) * r));
                    mesh.triangles.push_back({ size + 1, size + 2, size + 3 });
                }
                a += p.data[i].percentage;
            }

            // Create the text labels.
            p.size.textSize = math::Vector2i();
            p.draw.textLabels.clear();
            const int r2 = p.size.fontMetrics.lineHeight;
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                Private::DrawData::TextLabel label;
                label.text = p.data[i].text;
                label.size = event.fontSystem->getSize(label.text, fontInfo);
                label.pos.y = p.size.textSize.y;
                label.glyphs = event.fontSystem->getGlyphs(label.text, fontInfo);
                label.color = p.data[i].color;
                label.circleMesh = circle(math::Vector2i(r2 / 2, r2 / 2), r2 / 2, 60);
                p.draw.textLabels.push_back(label);
                p.size.textSize.x = std::max(
                    p.size.textSize.x,
                    r2 + p.size.spacing + label.size.x);
                p.size.textSize.y += p.size.fontMetrics.lineHeight;
                if (i < p.data.size() - 1)
                {
                    p.size.textSize.y += p.size.spacing;
                }
            }

            // Set the size hint.
            p.size.pieDiameter =
                p.size.fontMetrics.lineHeight * p.sizeMult +
                p.size.spacing * 2 +
                percentageWidthMax * 2;
            _sizeHint.x =
                p.size.pieDiameter +
                p.size.spacing +
                p.size.textSize.x +
                p.size.margin * 2;
            _sizeHint.y =
                std::max(p.size.pieDiameter, p.size.textSize.y) +
                p.size.margin * 2;
        }

        void PieChart::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            //event.render->drawRect(_geometry, image::Color4f(.5F, .3F, .3F));

            const math::Box2i g = align(
                _geometry.margin(-p.size.margin),
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign);

            // Draw the percentage labels.
            const math::Box2i g2(
                g.min.x,
                g.min.y,
                p.size.pieDiameter,
                p.size.pieDiameter);
            const math::Vector2i c = g2.getCenter();
            for (const auto& label : p.draw.percentageLabels)
            {
                event.render->drawText(
                    label.glyphs,
                    math::Vector2i(
                        c.x + label.pos.x,
                        c.y + label.pos.y + p.size.fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));
            }

            // Draw the pie slices.
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                event.render->drawMesh(
                    p.draw.pieSliceMeshes[i],
                    c,
                    p.data[i].color);
            }

            // Draw the text labels.
            math::Vector2i pos(
                g.min.x + p.size.pieDiameter + p.size.spacing,
                g.min.y + g.h() / 2 - p.size.textSize.y / 2);
            for (const auto& label : p.draw.textLabels)
            {
                event.render->drawMesh(
                    label.circleMesh,
                    math::Vector2i(
                        pos.x + label.pos.x,
                        pos.y + label.pos.y),
                    label.color);
                event.render->drawText(
                    label.glyphs,
                    math::Vector2i(
                        pos.x +
                        label.pos.x +
                        p.size.fontMetrics.lineHeight +
                        p.size.spacing,
                        pos.y +
                        label.pos.y +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));
            }
        }
    }
}
