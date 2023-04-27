// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
                const imaging::Color4f& color);

            std::string      text;
            int              percentage;
            imaging::Color4f color;

            bool operator == (const PieChartData&) const;
            bool operator != (const PieChartData&) const;
        };

        //! Pie chart widget.
        class PieChart : public IWidget
        {
            TLRENDER_NON_COPYABLE(PieChart);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            PieChart();

        public:
            ~PieChart() override;

            //! Create a new pie chart widget.
            static std::shared_ptr<PieChart> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the pie chart data.
            void setData(const std::vector<PieChartData>&);

            //! Set the font role.
            void setFontRole(FontRole);

            //! Set the size multiplier.
            void setSizeMult(int);

            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
