// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <string>
#include <vector>

namespace tl
{
    namespace file
    {
        //! File system path options.
        struct PathOptions
        {
            size_t maxNumberDigits = 9;

            bool operator == (const PathOptions&) const;
            bool operator != (const PathOptions&) const;
        };

        //! Does the given path end with a separator?
        bool hasEndSeparator(const std::string&);

        //! Add a path separator to the end if necessary.
        std::string appendSeparator(const std::string&);

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
            Path(
                const std::string& directory,
                const std::string& baseName,
                const std::string& number,
                uint8_t padding,
                const std::string& extension);

            //! Get the path.
            std::string get(int number = -1, bool directory = true) const;

            //! Get the directory.
            const std::string& getDirectory() const;

            //! Get the base name.
            const std::string& getBaseName() const;

            //! Get the number.
            const std::string& getNumber() const;

            //! Get the number zero padding.
            uint8_t getPadding() const;

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
            uint8_t _padding = 0;
            std::string _extension;
        };
    }
}

#include <tlCore/PathInline.h>