// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/PieChart.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/LayoutUtil.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace ui
    {
        PieChartData::PieChartData()
        {}

        PieChartData::PieChartData(
            const std::string& text,
            int percentage,
            const dtk::Color4F& color) :
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
                bool sizeInit = true;
                int margin = 0;
                int spacing = 0;
                dtk::FontInfo fontInfo;
                dtk::FontMetrics fontMetrics;

                int pieDiameter = 0;
                dtk::V2I textSize;
            };
            SizeData size;

            struct DrawData
            {
                struct PercentageLabel
                {
                    std::string text;
                    dtk::Size2I size;
                    dtk::V2I pos;
                    std::vector<std::shared_ptr<dtk::Glyph> > glyphs;
                };
                std::vector<PercentageLabel> percentageLabels;
                std::vector<dtk::TriMesh2F> pieSliceMeshes;
                struct TextLabel
                {
                    std::string text;
                    dtk::Size2I size;
                    dtk::V2I pos;
                    std::vector<std::shared_ptr<dtk::Glyph> > glyphs;
                    dtk::Color4F color;
                    dtk::TriMesh2F circleMesh;
                };
                std::vector<TextLabel> textLabels;
            };
            DrawData draw;
        };

        void PieChart::_init(
            const std::shared_ptr<dtk::Context>& context,
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PieChart>(new PieChart);
            out->_init(context, parent);
            return out;
        }

        void PieChart::setData(const std::vector<PieChartData>& value)
        {
            DTK_P();
            if (value == p.data)
                return;
            p.data = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void PieChart::setFontRole(FontRole value)
        {
            DTK_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void PieChart::setSizeMult(int value)
        {
            DTK_P();
            if (value == p.sizeMult)
                return;
            p.sizeMult = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void PieChart::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                p.size.fontInfo = event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            }
            p.size.sizeInit = false;

            // Create the percentage labels.
            p.draw.percentageLabels.clear();
            int percentageWidthMax = 0;
            for (const auto& data : p.data)
            {
                Private::DrawData::PercentageLabel label;
                label.text = dtk::Format("{0}%").arg(data.percentage);
                label.size = event.fontSystem->getSize(label.text, p.size.fontInfo);
                label.glyphs = event.fontSystem->getGlyphs(label.text, p.size.fontInfo);
                p.draw.percentageLabels.push_back(label);
                percentageWidthMax = std::max(percentageWidthMax, label.size.w);
            }
            int a = 0;
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                auto& label = p.draw.percentageLabels[i];
                const int d = a + p.data[i].percentage / 2;
                const float r = p.size.fontMetrics.lineHeight * p.sizeMult / 2.F +
                    p.size.spacing +
                    label.size.w / 2.F;
                label.pos.x = cos(dtk::deg2rad(d / 100.F * 360.F - 90.F)) * r -
                    label.size.w / 2;
                label.pos.y = sin(dtk::deg2rad(d / 100.F * 360.F - 90.F)) * r -
                    label.size.h / 2;
                a += p.data[i].percentage;
            }

            // Create the pie slices.
            p.draw.pieSliceMeshes.clear();
            p.draw.pieSliceMeshes.resize(p.data.size());
            const float r = p.size.fontMetrics.lineHeight * p.sizeMult / 2.F;
            a = 0;
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                dtk::TriMesh2F& mesh = p.draw.pieSliceMeshes[i];
                const int d = p.data[i].percentage;
                const int inc = 2;
                for (int i = a; i < a + d; i += inc)
                {
                    const size_t size = mesh.v.size();
                    mesh.v.push_back(dtk::V2F(0.F, 0.F));
                    mesh.v.push_back(dtk::V2F(
                        cos(dtk::deg2rad(i / 100.F * 360.F - 90.F)) * r,
                        sin(dtk::deg2rad(i / 100.F * 360.F - 90.F)) * r));
                    mesh.v.push_back(dtk::V2F(
                        cos(dtk::deg2rad(std::min(i + inc, a + d) / 100.F * 360.F - 90.F)) * r,
                        sin(dtk::deg2rad(std::min(i + inc, a + d) / 100.F * 360.F - 90.F)) * r));
                    mesh.triangles.push_back({ size + 1, size + 2, size + 3 });
                }
                a += p.data[i].percentage;
            }

            // Create the text labels.
            p.size.textSize = dtk::V2I();
            p.draw.textLabels.clear();
            const int r2 = p.size.fontMetrics.lineHeight;
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                Private::DrawData::TextLabel label;
                label.text = p.data[i].text;
                label.size = event.fontSystem->getSize(label.text, p.size.fontInfo);
                label.pos.y = p.size.textSize.y;
                label.glyphs = event.fontSystem->getGlyphs(label.text, p.size.fontInfo);
                label.color = p.data[i].color;
                label.circleMesh = circle(dtk::V2I(r2 / 2, r2 / 2), r2 / 2, 60);
                p.draw.textLabels.push_back(label);
                p.size.textSize.x = std::max(
                    p.size.textSize.x,
                    r2 + p.size.spacing + label.size.w);
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
            _sizeHint.w =
                p.size.pieDiameter +
                p.size.spacing +
                p.size.textSize.x +
                p.size.margin * 2;
            _sizeHint.h =
                std::max(p.size.pieDiameter, p.size.textSize.y) +
                p.size.margin * 2;
        }

        void PieChart::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            //event.render->drawRect(_geometry, dtk::Color4F(.5F, .3F, .3F));

            const dtk::Box2I g = align(
                margin(_geometry, -p.size.margin),
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign);

            // Draw the percentage labels.
            const dtk::Box2I g2(
                g.min.x,
                g.min.y,
                p.size.pieDiameter,
                p.size.pieDiameter);
            const dtk::V2I c = dtk::center(g2);
            for (const auto& label : p.draw.percentageLabels)
            {
                event.render->drawText(
                    label.glyphs,
                    p.size.fontMetrics,
                    c + label.pos,
                    event.style->getColorRole(ColorRole::Text));
            }

            // Draw the pie slices.
            for (size_t i = 0; i < p.data.size(); ++i)
            {
                event.render->drawMesh(
                    p.draw.pieSliceMeshes[i],
                    p.data[i].color,
                    dtk::V2F(c.x, c.y));
            }

            // Draw the text labels.
            dtk::V2I pos(
                g.min.x + p.size.pieDiameter + p.size.spacing,
                g.min.y + g.h() / 2 - p.size.textSize.y / 2);
            for (const auto& label : p.draw.textLabels)
            {
                event.render->drawMesh(
                    label.circleMesh,
                    label.color,
                    dtk::V2F(
                        pos.x + label.pos.x,
                        pos.y + label.pos.y));
                event.render->drawText(
                    label.glyphs,
                    p.size.fontMetrics,
                    dtk::V2I(
                        pos.x +
                        label.pos.x +
                        p.size.fontMetrics.lineHeight +
                        p.size.spacing,
                        pos.y +
                        label.pos.y),
                    event.style->getColorRole(ColorRole::Text));
            }
        }
    }
}
