// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/OpenWithAudioDialog.h>

#include <tlTimeline/Timeline.h>

#include <tlCore/String.h>

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

using namespace tl::core;

namespace tl
{
    namespace app
    {
        namespace play
        {
            struct OpenWithAudioDialog::Private
            {
                std::weak_ptr<system::Context> context;
                QString videoFileName;
                QString audioFileName;
                QLineEdit* videoLineEdit = nullptr;
                QLineEdit* audioLineEdit = nullptr;
            };

            OpenWithAudioDialog::OpenWithAudioDialog(
                const std::shared_ptr<system::Context>& context,
                QWidget* parent) :
                QDialog(parent),
                _p(new Private)
            {
                TLRENDER_P();

                p.context = context;

                setWindowTitle(tr("Open with Audio"));

                auto videoGroupBox = new QGroupBox(tr("Viedo"));
                p.videoLineEdit = new QLineEdit;
                auto videoBrowseButton = new QPushButton(tr("Browse"));

                auto audioGroupBox = new QGroupBox(tr("Audio"));
                p.audioLineEdit = new QLineEdit;
                auto audioBrowseButton = new QPushButton(tr("Browse"));

                auto buttonBox = new QDialogButtonBox;
                buttonBox->addButton(QDialogButtonBox::Ok);
                buttonBox->addButton(QDialogButtonBox::Cancel);

                auto layout = new QVBoxLayout;
                auto vLayout = new QVBoxLayout;
                auto hLayout = new QHBoxLayout;
                hLayout->addWidget(p.videoLineEdit);
                hLayout->addWidget(videoBrowseButton);
                videoGroupBox->setLayout(hLayout);
                vLayout->addWidget(videoGroupBox);
                hLayout = new QHBoxLayout;
                hLayout->addWidget(p.audioLineEdit);
                hLayout->addWidget(audioBrowseButton);
                audioGroupBox->setLayout(hLayout);
                vLayout->addWidget(audioGroupBox);
                layout->addLayout(vLayout);
                layout->addWidget(buttonBox);
                setLayout(layout);

                connect(
                    p.videoLineEdit,
                    SIGNAL(textChanged(const QString&)),
                    SLOT(_videoLineEditCallback(const QString&)));

                connect(
                    videoBrowseButton,
                    SIGNAL(clicked()),
                    SLOT(_browseVideoCallback()));

                connect(
                    p.audioLineEdit,
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

            OpenWithAudioDialog::~OpenWithAudioDialog()
            {}

            const QString& OpenWithAudioDialog::videoFileName() const
            {
                return _p->videoFileName;
            }

            const QString& OpenWithAudioDialog::audioFileName() const
            {
                return _p->audioFileName;
            }

            void OpenWithAudioDialog::_videoLineEditCallback(const QString& value)
            {
                _p->videoFileName = value.toUtf8().data();
            }

            void OpenWithAudioDialog::_browseVideoCallback()
            {
                TLRENDER_P();
                if (auto context = p.context.lock())
                {
                    std::vector<std::string> extensions;
                    for (const auto& i : timeline::getExtensions(
                        static_cast<int>(io::FileExtensionType::VideoAndAudio) |
                        static_cast<int>(io::FileExtensionType::VideoOnly),
                        context))
                    {
                        extensions.push_back("*" + i);
                    }

                    const auto fileName = QFileDialog::getOpenFileName(
                        this,
                        tr("Open Video"),
                        p.videoFileName,
                        tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
                    if (!fileName.isEmpty())
                    {
                        p.videoFileName = fileName;
                        p.videoLineEdit->setText(p.videoFileName);
                    }
                }
            }

            void OpenWithAudioDialog::_audioLineEditCallback(const QString& value)
            {
                TLRENDER_P();
                p.audioFileName = value.toUtf8().data();
            }

            void OpenWithAudioDialog::_browseAudioCallback()
            {
                TLRENDER_P();
                if (auto context = p.context.lock())
                {
                    std::vector<std::string> extensions;
                    for (const auto& i : timeline::getExtensions(
                        static_cast<int>(io::FileExtensionType::AudioOnly),
                        context))
                    {
                        extensions.push_back("*" + i);
                    }

                    const auto fileName = QFileDialog::getOpenFileName(
                        this,
                        tr("Open Audio"),
                        p.audioFileName,
                        tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
                    if (!fileName.isEmpty())
                    {
                        p.audioFileName = fileName;
                        p.audioLineEdit->setText(p.audioFileName);
                    }
                }
            }
        }
    }
}
