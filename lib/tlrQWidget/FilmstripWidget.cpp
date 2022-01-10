// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/FilmstripWidget.h>

#include <tlrQt/TimelineThumbnailProvider.h>

#include <tlrCore/Math.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>

namespace tlr
{
    namespace qwidget
    {
        struct FilmstripWidget::Private
        {
            std::shared_ptr<timeline::Timeline> timeline;
            int rowCount = 1;
            qt::TimelineThumbnailProvider* thumbnailProvider = nullptr;
            QSize thumbnailSize;
            struct Thumbnail
            {
                QImage image;
                std::chrono::steady_clock::time_point time;
            };
            std::map<otime::RationalTime, Thumbnail> thumbnails;
        };

        FilmstripWidget::FilmstripWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(50);
        }
        
        FilmstripWidget::~FilmstripWidget()
        {}

        void FilmstripWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            TLR_PRIVATE_P();
            p.timeline = timeline;
            if (p.thumbnailProvider)
            {
                delete p.thumbnailProvider;
                p.thumbnailProvider = nullptr;
            }
            if (p.timeline)
            {
                if (auto context = p.timeline->getContext().lock())
                {
                    p.thumbnailProvider = new qt::TimelineThumbnailProvider(
                        p.timeline,
                        context,
                        this);
                    connect(
                        p.thumbnailProvider,
                        SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                        SLOT(_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
                }
            }
            _thumbnailsUpdate();
        }

        void FilmstripWidget::setRowCount(int value)
        {
            TLR_PRIVATE_P();
            if (value == p.rowCount)
                return;
            p.rowCount = value;
            updateGeometry();
            _thumbnailsUpdate();
        }

        void FilmstripWidget::resizeEvent(QResizeEvent* event)
        {
            if (event->oldSize() != size())
            {
                _thumbnailsUpdate();
            }
        }

        void FilmstripWidget::paintEvent(QPaintEvent*)
        {
            TLR_PRIVATE_P();
            QPainter painter(this);
            const auto& rect = this->rect();
            painter.fillRect(rect, QColor(0, 0, 0));
            const int width = rect.width();
            int x = 0;
            int y = 0;
            const auto now = std::chrono::steady_clock::now();
            for (const auto& i : p.thumbnails)
            {
                const auto diff = std::chrono::duration<float>(now - i.second.time);
                const float opacity = math::lerp(diff.count(), 0.F, 1.F);
                if (opacity < 1.F)
                {
                    update();
                }
                painter.setOpacity(opacity);
                painter.drawImage(QPoint(x, y), i.second.image);
                x += p.thumbnailSize.width();
                if (x > width)
                {
                    x = 0;
                    y += p.thumbnailSize.height();
                }
            }
        }

        void FilmstripWidget::_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
        {
            TLR_PRIVATE_P();
            for (const auto& i : thumbnails)
            {
                Private::Thumbnail thumbnail;
                thumbnail.image = i.second;
                thumbnail.time = std::chrono::steady_clock::now();
                p.thumbnails[i.first] = thumbnail;

            }
            update();
        }

        void FilmstripWidget::_thumbnailsUpdate()
        {
            TLR_PRIVATE_P();
            p.thumbnails.clear();
            if (p.timeline && p.thumbnailProvider)
            {
                p.thumbnailProvider->cancelRequests();

                const auto& size = this->size();
                const int width = size.width();
                const int height = size.height();
                const auto& info = p.timeline->getAVInfo();
                const int thumbnailHeight = height / p.rowCount;
                const int thumbnailWidth = !info.video.empty() ?
                    static_cast<int>(thumbnailHeight * info.video[0].size.getAspect()) :
                    0;
                p.thumbnailSize = QSize(thumbnailWidth, thumbnailHeight);
                if (thumbnailWidth > 0)
                {
                    QList<otime::RationalTime> requests;
                    const auto& globalStartTime = p.timeline->getGlobalStartTime();
                    const auto& duration = p.timeline->getDuration();
                    const int count = static_cast<int>(ceilf(width / static_cast<float>(thumbnailWidth))) * p.rowCount;
                    for (int i = 0; i < count; ++i)
                    {
                        requests.push_back(otime::RationalTime(
                            floor(i / static_cast<double>(count) * (duration.value() - 1) + globalStartTime.value()),
                            duration.rate()));
                    }
                    p.thumbnailProvider->request(requests, QSize(thumbnailWidth, thumbnailHeight));
                }
            }
            update();
        }
    }
}
