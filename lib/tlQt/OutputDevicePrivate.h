// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/OutputDevice.h>

#include <tlDevice/IDeviceSystem.h>

#include <tlGl/GL.h>

#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace tl
{
    namespace qt
    {
        image::PixelType getOffscreenType(device::PixelType);
        GLenum getReadPixelsFormat(device::PixelType);
        GLenum getReadPixelsType(device::PixelType);
        GLint getReadPixelsAlign(device::PixelType);
        GLint getReadPixelsSwap(device::PixelType);

        class OverlayTexture
        {
            OverlayTexture(const QSize&, QImage::Format);

        public:
            ~OverlayTexture();

            static std::shared_ptr<OverlayTexture> create(const QSize&, QImage::Format);

            const QSize& getSize() const { return _size; }
            QImage::Format getFormat() const { return _format; }
            GLuint getID() const { return _id; }

            void copy(const QImage&);

        private:
            QSize _size;
            QImage::Format _format = QImage::Format::Format_Invalid;
            GLenum _textureFormat = GL_NONE;
            GLenum _textureType = GL_NONE;
            GLuint _id = 0;
        };

        struct OutputDevice::Private
        {
            std::weak_ptr<system::Context> context;
            std::weak_ptr<device::IDeviceSystem> deviceSystem;

            int deviceIndex = -1;
            int displayModeIndex = -1;
            device::PixelType pixelType = device::PixelType::_8BitBGRA;
            bool deviceEnabled = true;
            bool deviceActive = false;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            image::HDRData hdrData;

            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            QVector<QSharedPointer<qt::TimelinePlayer> > timelinePlayers;
            timeline::Playback playback = timeline::Playback::Stop;
            otime::RationalTime currentTime = time::invalidTime;
            std::vector<image::Size> sizes;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<QImage> overlay;
            float volume = 1.F;
            bool mute = false;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioData;

            std::chrono::milliseconds timeout = std::chrono::milliseconds(5);
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };
    }
}
