// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/OpenWithAudioDialog.h>

#include <tlCore/String.h>
#include <tlCore/Timeline.h>

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QPushButton>

namespace tl
{
    namespace play
    {
        OpenWithAudioDialog::OpenWithAudioDialog(
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            QDialog(parent),
            _context(context)
        {
            setWindowTitle(tr("Open with Audio"));

            auto videoGroupBox = new QGroupBox(tr("Viedo"));
            _videoLineEdit = new QLineEdit;
            auto videoBrowseButton = new QPushButton(tr("Browse"));

            auto audioGroupBox = new QGroupBox(tr("Audio"));
            _audioLineEdit = new QLineEdit;
            auto audioBrowseButton = new QPushButton(tr("Browse"));

            auto buttonBox = new QDialogButtonBox;
            buttonBox->addButton(QDialogButtonBox::Ok);
            buttonBox->addButton(QDialogButtonBox::Cancel);

            auto layout = new QVBoxLayout;
            auto vLayout = new QVBoxLayout;
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(_videoLineEdit);
            hLayout->addWidget(videoBrowseButton);
            videoGroupBox->setLayout(hLayout);
            vLayout->addWidget(videoGroupBox);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(_audioLineEdit);
            hLayout->addWidget(audioBrowseButton);
            audioGroupBox->setLayout(hLayout);
            vLayout->addWidget(audioGroupBox);
            layout->addLayout(vLayout);
            layout->addWidget(buttonBox);
            setLayout(layout);

            connect(
                _videoLineEdit,
                SIGNAL(textChanged(const QString&)),
                SLOT(_videoLineEditCallback(const QString&)));

            connect(
                videoBrowseButton,
                SIGNAL(clicked()),
                SLOT(_browseVideoCallback()));

            connect(
                _audioLineEdit,
                SIGNAL(textChanged(const QString&)),
                SLOT(_audioLineEditCallback(const QString&)));

            connect(
                audioBrowseButton,
                SIGNAL(clicked()),
                SLOT(_browseAudioCallback()));

            connect(
                buttonBox,
                SIGNAL(accepted()),
                SLOT(accept()));
            connect(
                buttonBox,
                SIGNAL(rejected()),
                SLOT(reject()));
        }

        const QString& OpenWithAudioDialog::videoFileName() const
        {
            return _videoFileName;
        }

        const QString& OpenWithAudioDialog::audioFileName() const
        {
            return _audioFileName;
        }

        void OpenWithAudioDialog::_videoLineEditCallback(const QString& value)
        {
            _videoFileName = value.toUtf8().data();
        }

        void OpenWithAudioDialog::_browseVideoCallback()
        {
            if (auto context = _context.lock())
            {
                std::vector<std::string> extensions;
                for (const auto& i : timeline::getExtensions(
                    static_cast<int>(avio::FileExtensionType::VideoAndAudio) |
                    static_cast<int>(avio::FileExtensionType::VideoOnly),
                    context))
                {
                    extensions.push_back("*" + i);
                }

                const auto fileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Open Video"),
                    _videoFileName,
                    tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
                if (!fileName.isEmpty())
                {
                    _videoFileName = fileName;
                    _videoLineEdit->setText(_videoFileName);
                }
            }
        }

        void OpenWithAudioDialog::_audioLineEditCallback(const QString& value)
        {
            _audioFileName = value.toUtf8().data();
        }

        void OpenWithAudioDialog::_browseAudioCallback()
        {
            if (auto context = _context.lock())
            {
                std::vector<std::string> extensions;
                for (const auto& i : timeline::getExtensions(
                    static_cast<int>(avio::FileExtensionType::AudioOnly),
                    context))
                {
                    extensions.push_back("*" + i);
                }

                const auto fileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Open Audio"),
                    _audioFileName,
                    tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
                if (!fileName.isEmpty())
                {
                    _audioFileName = fileName;
                    _audioLineEdit->setText(_audioFileName);
                }
            }
        }
    }
}
