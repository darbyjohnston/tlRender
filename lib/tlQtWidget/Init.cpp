// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Init.h>

#include <tlQt/Init.h>

#include <tlTimelineUI/Init.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/String.h>
#include <ftk/Core/Format.h>

#include <QDir>
#include <QFontDatabase>
#include <QMap>

#include <iostream>

namespace ftk_resource
{
    extern std::vector<uint8_t> NotoSansBold;
    extern std::vector<uint8_t> NotoMonoRegular;
    extern std::vector<uint8_t> NotoSansRegular;
}

namespace tl
{
    namespace qtwidget
    {
        void init(
            const std::shared_ptr<ftk::Context>& context,
            qt::DefaultSurfaceFormat defaultSurfaceFormat)
        {
            timelineui::init(context);
            qt::init(context, defaultSurfaceFormat);
        }

        void initFonts(const std::shared_ptr<ftk::Context>& context)
        {
            const std::vector<std::vector<uint8_t> > fonts =
            {
                ftk_resource::NotoMonoRegular,
                ftk_resource::NotoSansBold,
                ftk_resource::NotoSansRegular
            };
            std::vector<std::string> fontFamilyList;
            for (const auto& font : fonts)
            {
                const int id = QFontDatabase::addApplicationFontFromData(
                    QByteArray(reinterpret_cast<const char*>(font.data()), font.size()));
                for (const auto& j : QFontDatabase::applicationFontFamilies(id))
                {
                    fontFamilyList.push_back(j.toUtf8().data());
                }
            }
            context->log(
                "tl::qtwidget::initFonts",
                ftk::Format("Added Qt application fonts: {0}").arg(ftk::join(fontFamilyList, ", ")));
        }
    }
}
