// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <dtk/ui/DoubleEdit.h>
#include <dtk/ui/FormLayout.h>
#include <dtk/ui/RowLayout.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Cache settings widget.
        class CacheSettingsWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(CacheSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            CacheSettingsWidget() = default;

        public:
            ~CacheSettingsWidget();

            static std::shared_ptr<CacheSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            std::shared_ptr<dtk::DoubleEdit> _videoEdit;
            std::shared_ptr<dtk::DoubleEdit> _audioEdit;
            std::shared_ptr<dtk::DoubleEdit> _readBehindEdit;
            std::shared_ptr<dtk::FormLayout> _layout;
            std::shared_ptr<dtk::ValueObserver<timeline::PlayerCacheOptions> > _cacheObserver;
        };

        //! Settings widget.
        class SettingsWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(SettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            SettingsWidget() = default;

        public:
            ~SettingsWidget();

            static std::shared_ptr<SettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            std::shared_ptr<dtk::VerticalLayout> _layout;
        };
    }
}