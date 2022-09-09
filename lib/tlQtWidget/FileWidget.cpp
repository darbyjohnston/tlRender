// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtWidget/FileWidget.h>

#include <tlCore/Path.h>

#include <QBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QToolButton>

namespace tl
{
    namespace qtwidget
    {
        struct FileWidget::Private
        {
            QStringList extensions;
            std::string fileName;

            QLineEdit* lineEdit = nullptr;
        };

        FileWidget::FileWidget(
            const QStringList& extensions,
            QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.extensions = extensions;

            p.lineEdit = new QLineEdit;
            p.lineEdit->setToolTip(tr("File"));

            auto browseButton = new QToolButton;
            browseButton->setIcon(QIcon(":/Icons/FileBrowser.svg"));
            browseButton->setAutoRaise(true);
            browseButton->setToolTip(tr("Show the file browser"));

            auto clearButton = new QToolButton;
            clearButton->setIcon(QIcon(":/Icons/Clear.svg"));
            clearButton->setAutoRaise(true);
            clearButton->setToolTip(tr("Clear the file"));

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(1);
            layout->addWidget(p.lineEdit);
            layout->addWidget(browseButton);
            layout->addWidget(clearButton);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.lineEdit,
                &QLineEdit::editingFinished,
                [this]
                {
                    setFile(_p->lineEdit->text());
                });

            connect(
                browseButton,
                &QToolButton::clicked,
                [this]
                {
                    QString dir;
                    if (!_p->fileName.empty())
                    {
                        dir = QString::fromUtf8(file::Path(_p->fileName).get().c_str());
                    }

                    QString filter;
                    if (!_p->extensions.isEmpty())
                    {
                        filter.append(tr("Files"));
                        filter.append(" (");
                        QStringList extensions;
                        Q_FOREACH(QString i, _p->extensions)
                        {
                            extensions.push_back(QString("*%1").arg(i));
                        }
                        filter.append(extensions.join(' '));
                        filter.append(")");
                    }
                    const auto fileName = QFileDialog::getOpenFileName(
                        window(),
                        tr("Open"),
                        dir,
                        filter);
                    setFile(fileName);
                });

            connect(
                clearButton,
                &QToolButton::clicked,
                [this]
                {
                    clear();
                });
        }

        FileWidget::~FileWidget()
        {}

        void FileWidget::setFile(const QString& value)
        {
            TLRENDER_P();
            const std::string tmp = value.toUtf8().data();
            if (tmp == p.fileName)
                return;
            p.fileName = tmp;
            _widgetUpdate();
            Q_EMIT fileChanged(QString::fromUtf8(_p->fileName.c_str()));
        }

        void FileWidget::clear()
        {
            setFile(QString());
        }

        void FileWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.lineEdit);
                p.lineEdit->setText(QString::fromUtf8(p.fileName.c_str()));
            }
        }
    }
}
