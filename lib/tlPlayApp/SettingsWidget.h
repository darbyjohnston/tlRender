// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTimeline/Player.h>

#include <ftk/UI/DoubleEdit.h>
#include <ftk/UI/FormLayout.h>
#include <ftk/UI/RowLayout.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Cache settings widget.
        class CacheSettingsWidget : public ftk::IWidget
        {
            FTK_NON_COPYABLE(CacheSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            CacheSettingsWidget() = default;

        public:
            ~CacheSettingsWidget();

            static std::shared_ptr<CacheSettingsWidget> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            std::shared_ptr<ftk::DoubleEdit> _videoEdit;
            std::shared_ptr<ftk::DoubleEdit> _audioEdit;
            std::shared_ptr<ftk::DoubleEdit> _readBehindEdit;
            std::shared_ptr<ftk::FormLayout> _layout;
            std::shared_ptr<ftk::ValueObserver<timeline::PlayerCacheOptions> > _cacheObserver;
        };

        //! Settings widget.
        class SettingsWidget : public ftk::IWidget
        {
            FTK_NON_COPYABLE(SettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            SettingsWidget() = default;

        public:
            ~SettingsWidget();

            static std::shared_ptr<SettingsWidget> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            std::shared_ptr<ftk::VerticalLayout> _layout;
        };
    }
}