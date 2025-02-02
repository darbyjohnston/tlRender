// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QDialog>

#include <memory>

namespace dtk
{
    class Context;
}

namespace tl
{
    namespace play_qt
    {
        //! Open with separate audio dialog.
        class OpenSeparateAudioDialog : public QDialog
        {
            Q_OBJECT

        public:
            OpenSeparateAudioDialog(
                const std::shared_ptr<dtk::Context>&,
                QWidget* parent = nullptr);

            virtual ~OpenSeparateAudioDialog();

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
