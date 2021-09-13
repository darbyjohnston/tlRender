// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/FileIO.h>
#include <tlrCore/SequenceIO.h>

namespace tlr
{
    //! Cineon I/O.
    //!
    //! References:
    //! - Kodak, "4.5 DRAFT - Image File Format Proposal for Digital Pictures"
    namespace cineon
    {
        //! Cineon header magic numbers.
        const uint32_t magic[] =
        {
            0x802a5fd7,
            0xd75f2a80
        };

        //! Cineon image orientations.
        enum class Orient
        {
            LeftRightTopBottom,
            LeftRightBottomTop,
            RightLeftTopBottom,
            RightLeftBottomTop,
            TopBottomLeftRight,
            TopBottomRightLeft,
            BottomTopLeftRight,
            BottomTopRightLeft,

            Count,
            First = LeftRightTopBottom
        };
        TLR_ENUM(Orient);
        TLR_ENUM_SERIALIZE(Orient);

        //! Cineon file descriptors.
        enum class Descriptor
        {
            Luminance,
            RedFilmPrint,
            GreenFilmPrint,
            BlueFilmPrint,
            RedCCIRXA11,
            GreenCCIRXA11,
            BlueCCIRXA11,

            Count,
            First = Luminance
        };
        TLR_ENUM(Descriptor);
        TLR_ENUM_SERIALIZE(Descriptor);

        //! Cineon header.
        struct Header
        {
            Header();

            struct File
            {
                uint32_t magic;
                uint32_t imageOffset;
                uint32_t headerSize;
                uint32_t industryHeaderSize;
                uint32_t userHeaderSize;
                uint32_t size;
                char     version[8];
                char     name[100];
                char     time[24];
                uint8_t  pad[36];
            };
            File file;

            struct Image
            {
                uint8_t orient;
                uint8_t channels;
                uint8_t pad[2];

                struct Channel
                {
                    uint8_t  descriptor[2];
                    uint8_t  bitDepth;
                    uint8_t  pad;
                    uint32_t size[2];
                    float    lowData;
                    float    lowQuantity;
                    float    highData;
                    float    highQuantity;
                };
                Channel channel[8];

                float    white[2];
                float    red[2];
                float    green[2];
                float    blue[2];
                char     label[200];
                uint8_t  pad2[28];
                uint8_t  interleave;
                uint8_t  packing;
                uint8_t  dataSign;
                uint8_t  dataSense;
                uint32_t linePadding;
                uint32_t channelPadding;
                uint8_t  pad3[20];
            };
            Image image;

            struct Source
            {
                int32_t offset[2];
                char    file[100];
                char    time[24];
                char    inputDevice[64];
                char    inputModel[32];
                char    inputSerial[32];
                float   inputPitch[2];
                float   gamma;
                char    pad[40];
            };
            Source source;

            struct Film
            {
                uint8_t  id;
                uint8_t  type;
                uint8_t  offset;
                uint8_t  pad;
                uint8_t  prefix;
                uint32_t count;
                char     format[32];
                uint32_t frame;
                float    frameRate;
                char     frameId[32];
                char     slate[200];
                char     pad2[740];
            };
            Film film;
        };

        //! Check whether the Cineon header value is valid.
        bool isValid(const char*, size_t size);

        //! Convert a Cineon header string to std::string.
        std::string toString(const char*, size_t size);

        //! Convert a std::string to a Cineon header string.
        size_t fromString(
            const std::string& string,
            char*              out,
            size_t             maxLen,
            bool               terminate);

        //! Read a header.
        Header read(const std::shared_ptr<file::FileIO>&, avio::Info&);

        //! Write a header.
        void write(const std::shared_ptr<file::FileIO>&, const avio::Info&);

        //! Finish writing the header.
        void finishWrite(const std::shared_ptr<file::FileIO>&);

        //! Cineon reader.
        class Read : public avio::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);

        protected:
            avio::Info _getInfo(const std::string& fileName) override;
            avio::VideoFrame _readVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer,
                const std::shared_ptr<imaging::Image>&) override;
        };

        //! Cineon writer.
        class Write : public avio::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);

        protected:
            void _writeVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
        };

        //! Cineon plugin.
        class Plugin : public avio::IPlugin
        {
        protected:
            void _init(const std::shared_ptr<core::LogSystem>&);
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::shared_ptr<core::LogSystem>&);

            std::shared_ptr<avio::IRead> read(
                const file::Path&,
                const avio::Options& = avio::Options()) override;
            std::vector<imaging::PixelType> getWritePixelTypes() const override;
            uint8_t getWriteAlignment(imaging::PixelType) const override;
            memory::Endian getWriteEndian() const override;
            std::shared_ptr<avio::IWrite> write(
                const file::Path&,
                const avio::Info&,
                const avio::Options& = avio::Options()) override;
        };
    }
}
