// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Util.h>

#include <tlQt/Util.h>

#include <tlCore/Context.h>

#include <QDir>
#include <QFontDatabase>
#include <QMap>

#include <iostream>

namespace
{
#include <Fonts/NotoMono-Regular.font>
#include <Fonts/NotoSans-Regular.font>
#include <Fonts/NotoSans-Bold.font>
}

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

        namespace
        {
            QMap<QString, int> fonts;
        }

        QFont font(const QString& name)
        {
            if (fonts.isEmpty())
            {
                fonts["NotoMono-Regular"] = QFontDatabase::addApplicationFontFromData(
                    QByteArray(reinterpret_cast<const char*>(NotoMono_Regular_ttf), NotoMono_Regular_ttf_len));
                fonts["NotoSans-Bold"] = QFontDatabase::addApplicationFontFromData(
                    QByteArray(reinterpret_cast<const char*>(NotoSans_Bold_ttf), NotoSans_Bold_ttf_len));
                fonts["NotoSans-Regular"] = QFontDatabase::addApplicationFontFromData(
                    QByteArray(reinterpret_cast<const char*>(NotoSans_Regular_ttf), NotoSans_Regular_ttf_len));
            }
            QFont out;
            const auto i = fonts.find(name);
            if (i != fonts.end())
            {
                const auto families = QFontDatabase::applicationFontFamilies(i.value());
                if (!families.isEmpty())
                {
                    out = families.at(0);
                }
            }
            return out;
        }
    }
}
