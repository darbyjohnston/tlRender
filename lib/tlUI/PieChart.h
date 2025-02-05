// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Pie chart data.
        struct PieChartData
        {
            PieChartData();
            PieChartData(
                const std::string&      text,
                int                     percentage,
                const dtk::Color4F& color);

            std::string      text;
            int              percentage;
            dtk::Color4F color;

            bool operator == (const PieChartData&) const;
            bool operator != (const PieChartData&) const;
        };

        //! Pie chart widget.
        class PieChart : public IWidget
        {
            DTK_NON_COPYABLE(PieChart);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            PieChart();

        public:
            virtual ~PieChart();

            //! Create a new widget.
            static std::shared_ptr<PieChart> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the pie chart data.
            void setData(const std::vector<PieChartData>&);

            //! Set the font role.
            void setFontRole(FontRole);

            //! Set the size multiplier.
            void setSizeMult(int);

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
