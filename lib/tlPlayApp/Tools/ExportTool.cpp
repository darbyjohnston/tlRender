// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/ExportTool.h>

#include <tlPlayApp/App.h>

#include <dtk/ui/ComboBox.h>
#include <dtk/ui/FileEdit.h>
#include <dtk/ui/FormLayout.h>
#include <dtk/ui/IntEdit.h>
#include <dtk/ui/LineEdit.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

namespace tl
{
    namespace play
    {
        struct ExportTool::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::FileEdit> directoryEdit;
            std::shared_ptr<dtk::ComboBox> renderSizeComboBox;
            std::shared_ptr<dtk::IntEdit> renderWidthEdit;
            std::shared_ptr<dtk::IntEdit> renderHeightEdit;
            std::shared_ptr<dtk::ComboBox> fileTypeComboBox;
            std::shared_ptr<dtk::LineEdit> imageBaseNameEdit;
            std::shared_ptr<dtk::ComboBox> imageExtensionComboBox;
            std::shared_ptr<dtk::LineEdit> movieBaseNameEdit;
            std::shared_ptr<dtk::LineEdit> movieExtensionEdit;
            std::shared_ptr<dtk::ComboBox> movieCodecComboBox;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<ExportOptions> > optionsObserver;
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

            p.directoryEdit = dtk::FileEdit::create(context, dtk::FileBrowserMode::Dir);

            p.renderSizeComboBox = dtk::ComboBox::create(context, getExportRenderSizeLabels());
            p.renderWidthEdit = dtk::IntEdit::create(context);
            p.renderWidthEdit->setRange(dtk::RangeI(1, 16384));
            p.renderHeightEdit = dtk::IntEdit::create(context);
            p.renderHeightEdit->setRange(dtk::RangeI(1, 16384));

            p.layout = dtk::FormLayout::create(context);
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Directory:", p.directoryEdit);
            p.layout->addRow("Render size:", p.renderSizeComboBox);
            p.layout->addRow("Render width:", p.renderWidthEdit);
            p.layout->addRow("Render height:", p.renderHeightEdit);

            auto scrollWidget = dtk::ScrollWidget::create(context);
            scrollWidget->setBorder(false);
            scrollWidget->setWidget(p.layout);
            _setWidget(scrollWidget);

            p.optionsObserver = dtk::ValueObserver<ExportOptions>::create(
                p.model->observeExport(),
                [this](const ExportOptions& value)
                {
                    _widgetUpdate(value);
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
                    options.customRenderSize.w = value;
                    p.model->setExport(options);
                });

            p.renderHeightEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto options = p.model->getExport();
                    options.customRenderSize.h = value;
                    p.model->setExport(options);
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

        void ExportTool::_widgetUpdate(const ExportOptions& options)
        {
            DTK_P();
            p.directoryEdit->setPath(std::filesystem::u8path(options.directory));
            p.renderSizeComboBox->setCurrentIndex(static_cast<int>(options.renderSize));
            p.renderWidthEdit->setValue(options.customRenderSize.w);
            p.renderHeightEdit->setValue(options.customRenderSize.h);

            p.layout->setRowVisible(p.renderWidthEdit, ExportRenderSize::Custom == options.renderSize);
            p.layout->setRowVisible(p.renderHeightEdit, ExportRenderSize::Custom == options.renderSize);
        }
    }
}
