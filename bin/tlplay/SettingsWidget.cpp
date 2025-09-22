// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "SettingsWidget.h"

#include "App.h"
#include "SettingsModel.h"

#include <feather-tk/ui/GroupBox.h>

namespace tl
{
    namespace play
    {
        void CacheSettingsWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "CacheSettingsWidget", parent);

            _videoEdit = ftk::DoubleEdit::create(context);
            _videoEdit->setRange(0.0, 128.0);
            _videoEdit->setStep(1.0);
            _videoEdit->setLargeStep(10.0);

            _audioEdit = ftk::DoubleEdit::create(context);
            _audioEdit->setRange(0.0, 128.0);
            _audioEdit->setStep(1.0);
            _audioEdit->setLargeStep(10.0);

            _readAheadEdit = ftk::DoubleEdit::create(context);
            _readAheadEdit->setRange(0.0, 10.0);

            _readBehindEdit = ftk::DoubleEdit::create(context);
            _readBehindEdit->setRange(0.0, 2.0);

            _layout = ftk::FormLayout::create(context, shared_from_this());
            _layout->setSpacingRole(ftk::SizeRole::SpacingSmall);
            _layout->addRow("Video cache (GB):", _videoEdit);
            _layout->addRow("Audio cache (GB):", _audioEdit);
            _layout->addRow("Read ahead (seconds):", _readAheadEdit);
            _layout->addRow("Read behind (seconds):", _readBehindEdit);

            std::weak_ptr<App> appWeak(app);
            _videoEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.videoGB = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _audioEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.audioGB = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _readAheadEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.readAhead = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _readBehindEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.readBehind = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _cacheObserver = ftk::ValueObserver<timeline::PlayerCacheOptions>::create(
                app->getSettingsModel()->observeCache(),
                [this](const timeline::PlayerCacheOptions& value)
                {
                    _videoEdit->setValue(value.videoGB);
                    _audioEdit->setValue(value.audioGB);
                    _readAheadEdit->setValue(value.readAhead);
                    _readBehindEdit->setValue(value.readBehind);
                });
        }

        CacheSettingsWidget::~CacheSettingsWidget()
        {
        }

        std::shared_ptr<CacheSettingsWidget> CacheSettingsWidget::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CacheSettingsWidget>(new CacheSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void CacheSettingsWidget::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void CacheSettingsWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }

        void SettingsWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "SettingsWidget", parent);
            _layout = ftk::VerticalLayout::create(context, shared_from_this());
            _layout->setMarginRole(ftk::SizeRole::MarginSmall);
            _layout->setSpacingRole(ftk::SizeRole::SpacingSmall);
            auto groupBox = ftk::GroupBox::create(context, "Cache", _layout);
            CacheSettingsWidget::create(context, app, groupBox);
        }

        SettingsWidget::~SettingsWidget()
        {
        }

        std::shared_ptr<SettingsWidget> SettingsWidget::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SettingsWidget>(new SettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void SettingsWidget::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void SettingsWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }
    }
}