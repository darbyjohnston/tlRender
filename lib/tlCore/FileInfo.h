// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Path.h>

#include <nlohmann/json.hpp>

#include <iostream>

namespace tl
{
    namespace file
    {
        //! File types.
        enum class Type
        {
            File,      //!< Regular file
            Directory, //!< Directory

            Count,
            First = File
        };
        TLRENDER_ENUM(Type);
        TLRENDER_ENUM_SERIALIZE(Type);

        //! File permissions.
        enum class Permissions
        {
            Read  = 1, //!< Readable
            Write = 2, //!< Writable
            Exec  = 4, //!< Executable
        };

        //! File system information.
        class FileInfo
        {
        public:
            FileInfo();
            explicit FileInfo(const Path&);

            //! Get the path.
            const Path& getPath() const noexcept;

            //! Get the file type.
            Type getType() const noexcept;

            //! Get the file size.
            uint64_t getSize() const noexcept;

            //! Get the file permissions.
            int getPermissions() const noexcept;

            //! Get the file last modification time.
            time_t getTime() const noexcept;

        private:
            bool _stat(std::string* error);

            Path _path;
            bool _exists = false;
            Type _type = Type::File;
            uint64_t _size = 0;
            int _permissions = 0;
            time_t _time = 0;
        };

        //! Directory sorting.
        enum class ListSort
        {
            Name,
            Extension,
            Size,
            Time,

            Count,
            First = Name
        };
        TLRENDER_ENUM(ListSort);
        TLRENDER_ENUM_SERIALIZE(ListSort);

        //! Directory list options.
        struct ListOptions
        {
            ListSort sort                 = ListSort::Name;
            bool     reverseSort          = false;
            bool     sortDirectoriesFirst = true;
            bool     dotAndDotDotDirs     = false;
            bool     dotFiles             = false;
            bool     sequence             = true;
            bool     negativeNumbers      = false;
            size_t   maxNumberDigits      = 9;

            bool operator == (const ListOptions&) const;
            bool operator != (const ListOptions&) const;
        };

        //! Get the contents of the given directory.
        std::vector<FileInfo> list(
            const std::string&,
            const ListOptions & = ListOptions());

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const ListOptions&);

        void from_json(const nlohmann::json&, ListOptions&);

        ///@}
    }
}

#include <tlCore/FileInfoInline.h>
