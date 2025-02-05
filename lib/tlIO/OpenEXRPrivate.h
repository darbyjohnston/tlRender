// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/OpenEXR.h>

#include <dtk/core/Box.h>

#include <ImathBox.h>
#include <ImfHeader.h>
#include <ImfInputFile.h>
#include <ImfPixelType.h>

namespace tl
{
    namespace exr
    {
        //! Image channel.
        struct Channel
        {
            Channel();
            Channel(
                const std::string& name,
                Imf::PixelType     pixelType,
                const dtk::V2I&    sampling  = dtk::V2I(1, 1));

            std::string    name;
            Imf::PixelType pixelType = Imf::PixelType::HALF;
            dtk::V2I       sampling  = dtk::V2I(1, 1);
        };

        //! Image layer.
        struct Layer
        {
            Layer(
                const std::vector<Channel>& channels        = std::vector<Channel>(),
                bool                        luminanceChroma = false);

            std::string          name;
            std::vector<Channel> channels;
            bool                 luminanceChroma = false;
        };

        //! Convert to Imf.
        Imf::Compression toImf(Compression);

        //! Get a layer name from a list of channel names.
        std::string getLayerName(const std::vector<std::string>&);

        //! Get the channels that aren't in any layer.
        Imf::ChannelList getDefaultLayer(const Imf::ChannelList&);

        //! Find a channel by name.
        const Imf::Channel* find(const Imf::ChannelList&, std::string&);

        //! Get a list of layers from Imf channels.
        std::vector<Layer> getLayers(const Imf::ChannelList&, ChannelGrouping);

        //! Read the tags from an Imf header.
        void readTags(const Imf::Header&, dtk::ImageTags&);

        //! Write tags to an Imf header.
        //!
        //! \todo Write all the tags that are handled by readTags().
        void writeTags(const dtk::ImageTags&, double speed, Imf::Header&);

        //! Convert an Imath box type.
        dtk::Box2I fromImath(const Imath::Box2i&);

        //! Convert from an Imf channel.
        Channel fromImf(const std::string& name, const Imf::Channel&);

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
