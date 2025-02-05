// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/SettingsTool.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Cache settings widget.
        class CacheSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(CacheSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            CacheSettingsWidget();

        public:
            virtual ~CacheSettingsWidget();

            static std::shared_ptr<CacheSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };

        //! File sequences settings widget.
        class FileSequenceSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(FileSequenceSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            FileSequenceSettingsWidget();

        public:
            virtual ~FileSequenceSettingsWidget();

            static std::shared_ptr<FileSequenceSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };

#if defined(TLRENDER_FFMPEG)
        //! FFmpeg settings widget.
        class FFmpegSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(FFmpegSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            FFmpegSettingsWidget();

        public:
            virtual ~FFmpegSettingsWidget();

            static std::shared_ptr<FFmpegSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        //! USD settings widget.
        class USDSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(USDSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            USDSettingsWidget();

        public:
            virtual ~USDSettingsWidget();

            static std::shared_ptr<USDSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };
#endif // TLRENDER_USD

        //! File browser settings widget.
        class FileBrowserSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(FileBrowserSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            FileBrowserSettingsWidget();

        public:
            virtual ~FileBrowserSettingsWidget();

            static std::shared_ptr<FileBrowserSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };

        //! Performance settings widget.
        class PerformanceSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(PerformanceSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            PerformanceSettingsWidget();

        public:
            virtual ~PerformanceSettingsWidget();

            static std::shared_ptr<PerformanceSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };

        //! OpenGL settings widget.
        class OpenGLSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(OpenGLSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            OpenGLSettingsWidget();

        public:
            virtual ~OpenGLSettingsWidget();

            static std::shared_ptr<OpenGLSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };

        //! Style settings widget.
        class StyleSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(StyleSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            StyleSettingsWidget();

        public:
            virtual ~StyleSettingsWidget();

            static std::shared_ptr<StyleSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };

        //! Miscellaneous settings widget.
        class MiscSettingsWidget : public ui::IWidget
        {
            DTK_NON_COPYABLE(MiscSettingsWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            MiscSettingsWidget();

        public:
            virtual ~MiscSettingsWidget();

            static std::shared_ptr<MiscSettingsWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _settingsUpdate(const std::string&);

            DTK_PRIVATE();
        };
    }
}
