// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/OpenEXR.h>

#include <dtk/core/Box.h>

#include <ImathBox.h>
#include <ImfHeader.h>
#include <ImfPixelType.h>

namespace tl
{
    namespace exr
    {
        //! Read the tags from an Imf header.
        void readTags(const Imf::Header&, dtk::ImageTags&);

        //! Write tags to an Imf header.
        //!
        //! \todo Write all the tags that are handled by readTags().
        void writeTags(const dtk::ImageTags&, double speed, Imf::Header&);

        //! Convert to Imf.
        Imf::Compression toImf(Compression);

        //! Convert from Imath.
        dtk::Box2I fromImath(const Imath::Box2i&);

        //! Input stream.
        class IStream : public Imf::IStream
        {
            DTK_NON_COPYABLE(IStream);

        public:
            IStream(const std::string& fileName);
            IStream(const std::string& fileName, const uint8_t*, size_t);

            virtual ~IStream();

            bool isMemoryMapped() const override;
            char* readMemoryMapped(int n) override;
            bool read(char c[], int n) override;
            uint64_t tellg() override;
            void seekg(uint64_t pos) override;

        private:
            DTK_PRIVATE();
        };
   }
}
