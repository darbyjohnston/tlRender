// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/FilmstripWidget.h>

#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>

namespace tlr
{
    namespace qt
    {
        FilmstripWidget::FilmstripWidget(QWidget* parent) :
            QWidget(parent)
        {
            setMinimumHeight(50);

            _thumbnailThreadRunning = true;
            _thumbnailThread = std::thread(
                [this]
                {
                    while (_thumbnailThreadRunning)
                    {
                        bool request = false;
                        std::pair<io::VideoFrame, imaging::Size> data;
                        {
                            std::unique_lock<std::mutex> lock(_thumbnailMutex);
                            if (_thumbnailsCV.wait_for(
                                lock,
                                std::chrono::microseconds(1000),
                                [this]
                                {
                                    return !_thumbnailRequests.empty();
                                }))
                            {
                                request = true;
                                data = _thumbnailRequests.front();
                                _thumbnailRequests.pop_front();
                            }
                        }
                        if (request && data.first.image)
                        {
                            const auto& info = data.first.image->getInfo();
                            std::size_t scanlineByteCount = 0;
                            QImage::Format qFormat = QImage::Format_Invalid;
                            switch (info.pixelType)
                            {
                            case imaging::PixelType::L_U8:
                                scanlineByteCount = info.size.w;
                                qFormat = QImage::Format_Grayscale8;
                                break;
                            case imaging::PixelType::RGB_U8:
                                scanlineByteCount = info.size.w * 3;
                                qFormat = QImage::Format_RGB888;
                                break;
                            case imaging::PixelType::RGBA_U8:
                                scanlineByteCount = info.size.w * 4;
                                qFormat = QImage::Format_RGBA8888;
                                break;
                            case imaging::PixelType::RGBA_F16:
                                //! \todo Convert pixel types.
                                break;
                            }
                            if (qFormat != QImage::Format_Invalid)
                            {
                                const auto qImage = QImage(
                                    data.first.image->getData(),
                                    info.size.w,
                                    info.size.h,
                                    scanlineByteCount,
                                    qFormat);
                                const auto qImageScaled = qImage.scaled(QSize(data.second.w, data.second.h));
                                std::unique_lock<std::mutex> lock(_thumbnailMutex);
                                _thumbnailResults.push_back(std::make_pair(qImageScaled, data.first.time));;
                            }
                        }
                    }
                });

            startTimer(0);
        }

        FilmstripWidget::~FilmstripWidget()
        {
            _thumbnailThreadRunning = false;
            if (_thumbnailThread.joinable())
            {
                _thumbnailThread.join();
            }
        }

        void FilmstripWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            _timeline = timeline;
            _timelineUpdate();
        }

        void FilmstripWidget::resizeEvent(QResizeEvent* event)
        {
            if (event->oldSize() != size())
            {
                _timelineUpdate();
            }
        }

        void FilmstripWidget::paintEvent(QPaintEvent*)
        {
            QPainter painter(this);
            for (auto i = _thumbnails.begin(); i != _thumbnails.end(); ++i)
            {
                const int x = _timeToPos(i.key());
                painter.drawImage(QPoint(x, 0), i.value());
            }
        }

        void FilmstripWidget::timerEvent(QTimerEvent*)
        {
            bool results = false;
            {
                std::unique_lock<std::mutex> lock(_thumbnailMutex);
                results = !_thumbnailResults.empty();
                for (const auto& i : _thumbnailResults)
                {
                    _thumbnails[i.second] = i.first;
                }
                _thumbnailResults.clear();
            }
            if (results)
            {
                update();
            }
        }

        otime::RationalTime FilmstripWidget::_posToTime(int value) const
        {
            otime::RationalTime out;
            if (_timeline)
            {
                const double t = value / static_cast<double>(width());
                const auto& duration = _timeline->getDuration();
                out = otime::RationalTime(t * duration.value(), duration.rate());
            }
            return out;
        }

        int FilmstripWidget::_timeToPos(const otime::RationalTime& value) const
        {
            int out = 0;
            if (_timeline)
            {
                const auto& duration = _timeline->getDuration();
                const double t = value.value() / duration.value();
                out = static_cast<int>(width() * t);
            }
            return out;
        }

        void FilmstripWidget::_timelineUpdate()
        {
            _thumbnails.clear();
            if (_timeline)
            {
                const auto& duration = _timeline->getDuration();
                const auto& imageInfo = _timeline->getImageInfo();
                const auto& size = this->size();
                const int width = size.width();
                const int height = size.height();
                _thumbnailSize.w = static_cast<int>(height * imageInfo.size.getAspect());
                _thumbnailSize.h = height;
                if (_thumbnailSize.w > 0)
                {
                    int x = 0;
                    while (x < width)
                    {
                        //_times.push_back(_posToTime(x));
                        x += _thumbnailSize.w;
                    }
                }
            }
            update();
        }
    }
}
