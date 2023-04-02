// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/GridLayout.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            struct GridPos
            {
                int row = 0;
                int column = 0;
            };
        }

        struct GridLayout::Private
        {
            std::map<std::shared_ptr<IWidget>, GridPos> gridPos;
            SizeRole marginRole = SizeRole::None;
            int margin = 0;
            SizeRole spacingRole = SizeRole::Spacing;
            int spacing = 0;

            GridPos getSize() const;

            void getSizeHints(
                std::vector<int>& rows,
                std::vector<int>& columns) const;

            void getStretch(
                std::vector<bool>& rows,
                std::vector<bool>& columns) const;
        };

        void GridLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::GridLayout", context, parent);
        }

        GridLayout::GridLayout() :
            _p(new Private)
        {}

        GridLayout::~GridLayout()
        {}

        std::shared_ptr<GridLayout> GridLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GridLayout>(new GridLayout);
            out->_init(context, parent);
            return out;
        }

        void GridLayout::setGridPos(
            const std::shared_ptr<IWidget>& child,
            int row,
            int column)
        {
            TLRENDER_P();
            p.gridPos[child].row = row;
            p.gridPos[child].column = column;
            _updates |= Update::Size;
        }

        void GridLayout::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
        }

        void GridLayout::setSpacingRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.spacingRole)
                return;
            p.spacingRole = value;
            _updates |= Update::Size;
        }

        void GridLayout::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            math::BBox2i g = _geometry.margin(-p.margin);

            // Get the child size hints.
            std::vector<int> rowSizeHints;
            std::vector<int> columnSizeHints;
            p.getSizeHints(rowSizeHints, columnSizeHints);

            // Get the total size.
            math::Vector2i totalSize;
            for (int i = 0; i < rowSizeHints.size(); ++i)
            {
                totalSize.y += rowSizeHints[i];
                if (i < rowSizeHints.size() - 1)
                {
                    totalSize.y += p.spacing;
                }
            }
            for (int i = 0; i < columnSizeHints.size(); ++i)
            {
                totalSize.x += columnSizeHints[i];
                if (i < columnSizeHints.size() - 1)
                {
                    totalSize.x += p.spacing;
                }
            }

            // Get the layout stretch.
            std::vector<bool> rowStretch;
            std::vector<bool> columnStretch;
            p.getStretch(rowStretch, columnStretch);

            // Get the layout stretch size.
            size_t rowStretchCount = 0;
            size_t columnStretchCount = 0;
            for (int i : rowStretch)
            {
                if (i)
                {
                    ++rowStretchCount;
                }
            }
            for (int i : columnStretch)
            {
                if (i)
                {
                    ++columnStretchCount;
                }
            }
            math::Vector2i stretchSize;
            if (rowStretchCount > 0)
            {
                stretchSize.y = (g.h() - totalSize.y) / rowStretchCount;
            }
            if (columnStretchCount > 0)
            {
                stretchSize.x = (g.w() - totalSize.x) / columnStretchCount;
            }

            // Get the sizes.
            std::vector<int> rowSizes;
            std::vector<int> columnSizes;
            for (int i = 0; i < rowSizeHints.size(); ++i)
            {
                int size = rowSizeHints[i];
                if (rowStretch[i])
                {
                    size += stretchSize.y;
                }
                rowSizes.push_back(size);
            }
            for (int i = 0; i < columnSizeHints.size(); ++i)
            {
                int size = columnSizeHints[i];
                if (columnStretch[i])
                {
                    size += stretchSize.x;
                }
                columnSizes.push_back(size);
            }

            // Layout the children.
            for (const auto& i : p.gridPos)
            {
                math::Vector2i pos = g.min;
                for (int j = 0; j < i.second.row; ++j)
                {
                    pos.y += rowSizes[j] + p.spacing;
                }
                for (int j = 0; j < i.second.column; ++j)
                {
                    pos.x += columnSizes[j] + p.spacing;
                }
                const math::Vector2i size(
                    columnSizes[i.second.column],
                    rowSizes[i.second.row]);
                i.first->setGeometry(math::BBox2i(pos.x, pos.y, size.x, size.y));
            }
        }

        void GridLayout::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(p.marginRole) * event.contentScale;
            p.spacing = event.style->getSizeRole(p.spacingRole) * event.contentScale;

            // Get size hints.
            std::vector<int> rowSizeHints;
            std::vector<int> columnSizeHints;
            p.getSizeHints(rowSizeHints, columnSizeHints);
            _sizeHint.y = 0;
            _sizeHint.x = 0;
            for (int i : rowSizeHints)
            {
                _sizeHint.y += i;
            }
            for (int i : columnSizeHints)
            {
                _sizeHint.x += i;
            }

            // Add spacing.
            if (!rowSizeHints.empty())
            {
                _sizeHint.y += (rowSizeHints.size() - 1) * p.spacing;
            }
            if (!columnSizeHints.empty())
            {
                _sizeHint.x += (columnSizeHints.size() - 1) * p.spacing;
            }

            // Add the margin.
            _sizeHint.x += p.margin * 2;
            _sizeHint.y += p.margin * 2;
        }

        void GridLayout::childRemovedEvent(const ChildEvent& event)
        {
            TLRENDER_P();
            const auto i = p.gridPos.find(event.child);
            if (i != p.gridPos.end())
            {
                p.gridPos.erase(i);
            }
            _updates |= Update::Size;
        }

        GridPos GridLayout::Private::getSize() const
        {
            GridPos out;
            for (const auto& i : gridPos)
            {
                out.row = std::max(out.row, i.second.row);
                out.column = std::max(out.column, i.second.column);
            }
            if (!gridPos.empty())
            {
                out.row += 1;
                out.column += 1;
            }
            return out;
        }

        void GridLayout::Private::getSizeHints(
            std::vector<int>& rows,
            std::vector<int>& columns) const
        {
            const GridPos size = getSize();
            rows = std::vector<int>(size.row, false);
            columns = std::vector<int>(size.column, false);
            for (const auto& i : gridPos)
            {
                const math::Vector2i& sizeHint = i.first->getSizeHint();
                rows[i.second.row] = std::max(rows[i.second.row], sizeHint.y);
                columns[i.second.column] = std::max(columns[i.second.column], sizeHint.x);
            }
        }

        void GridLayout::Private::getStretch(
            std::vector<bool>& rows,
            std::vector<bool>& columns) const
        {
            const GridPos size = getSize();
            rows = std::vector<bool>(size.row, false);
            columns = std::vector<bool>(size.column, false);
            for (const auto& i : gridPos)
            {
                if (Stretch::Expanding == i.first->getVStretch())
                {
                    rows[i.second.row] = true;
                }
                if (Stretch::Expanding == i.first->getHStretch())
                {
                    columns[i.second.column] = true;
                }
            }
        }
    }
}
