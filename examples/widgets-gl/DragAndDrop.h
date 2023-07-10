// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            class DragAndDropData : public ui::DragAndDropData
            {
            public:
                DragAndDropData(int);

                ~DragAndDropData() override;

                int getNumber() const;

            private:
                int _number = 0;
            };

            class DragAndDropWidget : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(DragAndDropWidget);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                DragAndDropWidget();

            public:
                ~DragAndDropWidget() override;

                static std::shared_ptr<DragAndDropWidget> create(
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
                void dragEnterEvent(ui::DragAndDropEvent&) override;
                void dragLeaveEvent(ui::DragAndDropEvent&) override;
                void dropEvent(ui::DragAndDropEvent&) override;

            private:
                void _textUpdate();

                TLRENDER_PRIVATE();
            };

            //! Drag and drop.
            class DragAndDrop : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(DragAndDrop);

            protected:
                void _init(const std::shared_ptr<system::Context>&);

                DragAndDrop();

            public:
                ~DragAndDrop();

                static std::shared_ptr<DragAndDrop> create(
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
