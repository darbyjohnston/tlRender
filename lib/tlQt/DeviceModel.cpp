// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/DeviceModel.h>

namespace tl
{
    namespace qt
    {
        struct DeviceModel::Private
        {
        };

        DeviceModel::DeviceModel(QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();
        }

        DeviceModel::~DeviceModel()
        {}
    }
}
