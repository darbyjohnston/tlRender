// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QObject>

#include <memory>

namespace tl
{
    namespace qt
    {
        //! Device model.
        class DeviceModel : public QObject
        {
            Q_OBJECT

        public:
            DeviceModel(QObject* parent = nullptr);

            ~DeviceModel() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
