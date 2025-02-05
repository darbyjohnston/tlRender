// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            class DragAndDropData : public ui::DragAndDropData
            {
            public:
                DragAndDropData(int);

                virtual ~DragAndDropData();

                int getNumber() const;

            private:
                int _number = 0;
            };

            class DragAndDropWidget : public ui::IWidget
            {
                DTK_NON_COPYABLE(DragAndDropWidget);

            protected:
                void _init(
                    int number,
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                DragAndDropWidget();

            public:
                virtual ~DragAndDropWidget();

                static std::shared_ptr<DragAndDropWidget> create(
                    int number,
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;
                void drawEvent(const dtk::Box2I&, const ui::DrawEvent&) override;
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

                DTK_PRIVATE();
            };

            //! Drag and drop.
            class DragAndDrop : public IExampleWidget
            {
                DTK_NON_COPYABLE(DragAndDrop);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                DragAndDrop();

            public:
                ~DragAndDrop();

                static std::shared_ptr<DragAndDrop> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                DTK_PRIVATE();
            };
        }
    }
}
