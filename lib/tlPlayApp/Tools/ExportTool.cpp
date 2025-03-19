// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/ExportTool.h>

#include <tlPlayApp/App.h>

#include <tlIO/System.h>
#if defined(TLRENDER_FFMPEG)
#include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG

#include <dtk/ui/ComboBox.h>
#include <dtk/ui/FileEdit.h>
#include <dtk/ui/FormLayout.h>
#include <dtk/ui/IntEdit.h>
#include <dtk/ui/LineEdit.h>
#include <dtk/ui/PushButton.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

namespace tl
{
    namespace play
    {
        struct ExportTool::Private
        {
            std::shared_ptr<SettingsModel> model;
            std::vector<std::string> imageExtensions;
            std::vector<std::string> movieExtensions;
            std::vector<std::string> movieCodecs;

            std::shared_ptr<dtk::FileEdit> directoryEdit;
            std::shared_ptr<dtk::ComboBox> renderSizeComboBox;
            std::shared_ptr<dtk::IntEdit> renderWidthEdit;
            std::shared_ptr<dtk::IntEdit> renderHeightEdit;
            std::shared_ptr<dtk::ComboBox> fileTypeComboBox;
            std::shared_ptr<dtk::LineEdit> imageBaseNameEdit;
            std::shared_ptr<dtk::IntEdit> imageZeroPadEdit;
            std::shared_ptr<dtk::ComboBox> imageExtensionComboBox;
            std::shared_ptr<dtk::LineEdit> movieBaseNameEdit;
            std::shared_ptr<dtk::ComboBox> movieExtensionComboBox;
            std::shared_ptr<dtk::ComboBox> movieCodecComboBox;
            std::shared_ptr<dtk::PushButton> exportButton;
            std::shared_ptr<dtk::HorizontalLayout> customSizeLayout;
            std::shared_ptr<dtk::FormLayout> formLayout;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<ExportSettings> > settingsObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void ExportTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Export,
                "tl::play_app::ExportTool",
                parent);
            DTK_P();

            p.model = app->getSettingsModel();

            auto ioSystem = context->getSystem<io::WriteSystem>();
            auto extensions = ioSystem->getExtensions(static_cast<int>(io::FileType::Sequence));
            p.imageExtensions.insert(p.imageExtensions.end(), extensions.begin(), extensions.end());
            extensions = ioSystem->getExtensions(static_cast<int>(io::FileType::Movie));
            p.movieExtensions.insert(p.movieExtensions.end(), extensions.begin(), extensions.end());
#if defined(TLRENDER_FFMPEG)
            auto ffmpegPlugin = ioSystem->getPlugin<ffmpeg::WritePlugin>();
            p.movieCodecs = ffmpegPlugin->getCodecs();
#endif // TLRENDER_FFMPEG

            p.directoryEdit = dtk::FileEdit::create(context, dtk::FileBrowserMode::Dir);

            p.renderSizeComboBox = dtk::ComboBox::create(context, getExportRenderSizeLabels());
            p.renderWidthEdit = dtk::IntEdit::create(context);
            p.renderWidthEdit->setRange(dtk::RangeI(1, 16384));
            p.renderHeightEdit = dtk::IntEdit::create(context);
            p.renderHeightEdit->setRange(dtk::RangeI(1, 16384));

            p.fileTypeComboBox = dtk::ComboBox::create(context, getExportFileTypeLabels());

            p.imageBaseNameEdit = dtk::LineEdit::create(context);
            p.imageZeroPadEdit = dtk::IntEdit::create(context);
            p.imageZeroPadEdit->setRange(dtk::RangeI(0, 16));
            p.imageExtensionComboBox = dtk::ComboBox::create(context, p.imageExtensions);

            p.movieBaseNameEdit = dtk::LineEdit::create(context);
            p.movieExtensionComboBox = dtk::ComboBox::create(context, p.movieExtensions);
            p.movieCodecComboBox = dtk::ComboBox::create(context, p.movieCodecs);

            p.exportButton = dtk::PushButton::create(context, "Export");

            p.layout = dtk::VerticalLayout::create(context);
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.formLayout = dtk::FormLayout::create(context, p.layout);
            p.formLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.formLayout->addRow("Directory:", p.directoryEdit);
            p.formLayout->addRow("Render size:", p.renderSizeComboBox);
            p.customSizeLayout = dtk::HorizontalLayout::create(context);
            p.customSizeLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.renderWidthEdit->setParent(p.customSizeLayout);
            p.renderHeightEdit->setParent(p.customSizeLayout);
            p.formLayout->addRow("Custom size:", p.customSizeLayout);
            p.formLayout->addRow("File type:", p.fileTypeComboBox);
            p.formLayout->addRow("Zero padding:", p.imageZeroPadEdit);
            p.formLayout->addRow("Base name:", p.imageBaseNameEdit);
            p.formLayout->addRow("Extension:", p.imageExtensionComboBox);
            p.formLayout->addRow("Base name:", p.movieBaseNameEdit);
            p.formLayout->addRow("Extension:", p.movieExtensionComboBox);
            p.formLayout->addRow("Codec:", p.movieCodecComboBox);
            p.exportButton->setParent(p.layout);

            auto scrollWidget = dtk::ScrollWidget::create(context);
            scrollWidget->setBorder(false);
            scrollWidget->setWidget(p.layout);
            _setWidget(scrollWidget);

            p.settingsObserver = dtk::ValueObserver<ExportSettings>::create(
                p.model->observeExport(),
                [this](const ExportSettings& value)
                {
                    _widgetUpdate(value);
                });

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _p->exportButton->setEnabled(value.get());
                });

            p.directoryEdit->setCallback(
                [this](const std::filesystem::path& value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.directory = value.u8string();
                    p.model->setExport(options);
                });

            p.renderSizeComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.renderSize = static_cast<ExportRenderSize>(value);
                    p.model->setExport(options);
                });

            p.renderWidthEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.customSize.w = value;
                    p.model->setExport(options);
                });

            p.renderHeightEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.customSize.h = value;
                    p.model->setExport(options);
                });

            p.fileTypeComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.fileType = static_cast<ExportFileType>(value);
                    p.model->setExport(options);
                });

            p.imageBaseNameEdit->setTextCallback(
                [this](const std::string& value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.imageBaseName = value;
                    p.model->setExport(options);
                });

            p.imageZeroPadEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.imageZeroPad = value;
                    p.model->setExport(options);
                });

            p.imageExtensionComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    if (value >= 0 && value < p.imageExtensions.size())
                    {
                        auto options = p.model->getExport();
                        options.imageExtension = p.imageExtensions[value];
                        p.model->setExport(options);
                    }
                });

            p.movieBaseNameEdit->setTextCallback(
                [this](const std::string& value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.movieBaseName = value;
                    p.model->setExport(options);
                });

            p.movieExtensionComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    if (value >= 0 && value < p.movieExtensions.size())
                    {
                        auto options = p.model->getExport();
                        options.movieExtension = p.movieExtensions[value];
                        p.model->setExport(options);
                    }
                });

            p.movieCodecComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    if (value >= 0 && value < p.movieCodecs.size())
                    {
                        auto options = p.model->getExport();
                        options.movieCodec = p.movieCodecs[value];
                        p.model->setExport(options);
                    }
                });
        }

        ExportTool::ExportTool() :
            _p(new Private)
        {}

        ExportTool::~ExportTool()
        {}

        std::shared_ptr<ExportTool> ExportTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ExportTool>(new ExportTool);
            out->_init(context, app, parent);
            return out;
        }

        void ExportTool::_widgetUpdate(const ExportSettings& settings)
        {
            DTK_P();
            p.directoryEdit->setPath(std::filesystem::u8path(settings.directory));
            p.renderSizeComboBox->setCurrentIndex(static_cast<int>(settings.renderSize));
            p.renderWidthEdit->setValue(settings.customSize.w);
            p.renderHeightEdit->setValue(settings.customSize.h);
            p.fileTypeComboBox->setCurrentIndex(static_cast<int>(settings.fileType));
            p.imageBaseNameEdit->setText(settings.imageBaseName);
            auto i = std::find(p.imageExtensions.begin(), p.imageExtensions.end(), settings.imageExtension);
            p.imageExtensionComboBox->setCurrentIndex(i != p.imageExtensions.end() ? (i - p.imageExtensions.begin()) : -1);
            p.movieBaseNameEdit->setText(settings.movieBaseName);
            i = std::find(p.movieExtensions.begin(), p.movieExtensions.end(), settings.movieExtension);
            p.movieExtensionComboBox->setCurrentIndex(i != p.movieExtensions.end() ? (i - p.movieExtensions.begin()) : -1);
            i = std::find(p.movieCodecs.begin(), p.movieCodecs.end(), settings.movieCodec);
            p.movieCodecComboBox->setCurrentIndex(i != p.movieCodecs.end() ? (i - p.movieCodecs.begin()) : -1);

            p.formLayout->setRowVisible(p.customSizeLayout, ExportRenderSize::Custom == settings.renderSize);
            p.formLayout->setRowVisible(p.imageBaseNameEdit, ExportFileType::Images == settings.fileType);
            p.formLayout->setRowVisible(p.imageZeroPadEdit, ExportFileType::Images == settings.fileType);
            p.formLayout->setRowVisible(p.imageExtensionComboBox, ExportFileType::Images == settings.fileType);
            p.formLayout->setRowVisible(p.movieBaseNameEdit, ExportFileType::Movie == settings.fileType);
            p.formLayout->setRowVisible(p.movieExtensionComboBox, ExportFileType::Movie == settings.fileType);
            p.formLayout->setRowVisible(p.movieCodecComboBox, ExportFileType::Movie == settings.fileType);
        }
    }
}
