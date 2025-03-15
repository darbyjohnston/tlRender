// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools/ViewTool.h>

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ForegroundOptions.h>

namespace tl
{
    namespace play
    {
        class ViewOptionsWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(ViewOptionsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            ViewOptionsWidget();

        public:
            virtual ~ViewOptionsWidget();

            static std::shared_ptr<ViewOptionsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };

        class BackgroundWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(BackgroundWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            BackgroundWidget();

        public:
            virtual ~BackgroundWidget();

            static std::shared_ptr<BackgroundWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            void _optionsUpdate(const timeline::BackgroundOptions&);

            DTK_PRIVATE();
        };

        class OutlineWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(OutlineWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            OutlineWidget();

        public:
            virtual ~OutlineWidget();

            static std::shared_ptr<OutlineWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            void _optionsUpdate(const timeline::BackgroundOptions&);

            DTK_PRIVATE();
        };

        class GridWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(GridWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            GridWidget();

        public:
            virtual ~GridWidget();

            static std::shared_ptr<GridWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };

        class HUDWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(HUDWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            HUDWidget();

        public:
            virtual ~HUDWidget();

            static std::shared_ptr<HUDWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
