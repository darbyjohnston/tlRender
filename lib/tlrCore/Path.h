// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <string>
#include <vector>

namespace tlr
{
    namespace file
    {
        //! File system path options.
        struct PathOptions
        {
            uint8_t maxNumberDigits = 9;
        };

        //! File system path.
        class Path
        {
        public:
            Path();
            explicit Path(
                const std::string&,
                const PathOptions& = PathOptions());
            Path(
                const std::string&,
                const std::string&,
                const PathOptions& = PathOptions());

            //! Get the path.
            std::string get(int number = -1, bool directory = true) const;

            //! Get the directory.
            const std::string& getDirectory() const;

            //! Get the base name.
            const std::string& getBaseName() const;

            //! Get the number.
            const std::string& getNumber() const;

            //! Get the number zero padding.
            int getPadding() const;

            //! Get the extension.
            const std::string& getExtension() const;

            //! Is the path empty?
            bool isEmpty() const;

            //! Is the path absolute?
            bool isAbsolute() const;

            bool operator == (const Path&) const;
            bool operator != (const Path&) const;

        private:
            std::string _directory;
            std::string _baseName;
            std::string _number;
            int _padding = 0;
            std::string _extension;
        };
    }
}

#include <tlrCore/PathInline.h>