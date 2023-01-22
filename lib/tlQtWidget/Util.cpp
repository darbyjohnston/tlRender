// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Util.h>

#include <tlQt/Util.h>

#include <tlCore/Context.h>
#include <tlCore/FontSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <QDir>
#include <QFontDatabase>
#include <QMap>

#include <iostream>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlQtWidget);
}

namespace tl
{
    namespace qtwidget
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            qt::init(context);

            qtInitResources();
        }

        void initFonts(const std::shared_ptr<system::Context>& context)
        {
            std::vector<std::string> fontFamilyList;
            for (const auto& i : std::vector<std::string>(
                {
                    "NotoMono-Regular",
                    "NotoSans-Bold",
                    "NotoSans-Regular"
                }))
            {
                const auto font = imaging::getFontData(i);
                const int id = QFontDatabase::addApplicationFontFromData(
                    QByteArray(reinterpret_cast<const char*>(font.data()), font.size()));
                for (const auto& j : QFontDatabase::applicationFontFamilies(id))
                {
                    fontFamilyList.push_back(j.toUtf8().data());
                }
            }
            context->log(
                "tl::qtwidget::initFonts",
                string::Format("Added Qt application fonts: {0}").arg(string::join(fontFamilyList, ", ")));
        }
    }
}
