// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Context.h>

#include <QDialog>
#include <QLineEdit>

namespace tlr
{
    //! Open with audio dialog.
    class OpenWithAudioDialog : public QDialog
    {
        Q_OBJECT

    public:
        OpenWithAudioDialog(
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

        const QString& videoFileName() const;
        const QString& audioFileName() const;

    private Q_SLOTS:
        void _videoLineEditCallback(const QString&);
        void _browseVideoCallback();
        void _audioLineEditCallback(const QString&);
        void _browseAudioCallback();

    private:
        std::weak_ptr<core::Context> _context;
        QString _videoFileName;
        QString _audioFileName;
        QLineEdit* _videoLineEdit = nullptr;
        QLineEdit* _audioLineEdit = nullptr;
    };
}
