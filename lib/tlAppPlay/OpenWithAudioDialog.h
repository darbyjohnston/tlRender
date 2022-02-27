// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QDialog>

#include <memory>

namespace tl
{
    namespace core
    {
        namespace system
        {
            class Context;
        }
    }

    namespace app
    {
        namespace play
        {
            //! Open with audio dialog.
            class OpenWithAudioDialog : public QDialog
            {
                Q_OBJECT

            public:
                OpenWithAudioDialog(
                    const std::shared_ptr<core::system::Context>&,
                    QWidget* parent = nullptr);

                ~OpenWithAudioDialog() override;

                const QString& videoFileName() const;
                const QString& audioFileName() const;

            private Q_SLOTS:
                void _videoLineEditCallback(const QString&);
                void _browseVideoCallback();
                void _audioLineEditCallback(const QString&);
                void _browseAudioCallback();

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
