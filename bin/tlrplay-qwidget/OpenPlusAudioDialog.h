// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Context.h>

#include <QDialog>
#include <QLineEdit>

namespace tlr
{
    //! Open plus audio dialog.
    class OpenPlusAudioDialog : public QDialog
    {
        Q_OBJECT

    public:
        OpenPlusAudioDialog(
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

        const QString& mediaFileName() const;
        const QString& audioFileName() const;

    private Q_SLOTS:
        void _mediaLineEditCallback(const QString&);
        void _browseMediaCallback();
        void _audioLineEditCallback(const QString&);
        void _browseAudioCallback();

    private:
        std::weak_ptr<core::Context> _context;
        QString _mediaFileName;
        QString _audioFileName;
        QLineEdit* _mediaLineEdit = nullptr;
        QLineEdit* _audioLineEdit = nullptr;
    };
}
