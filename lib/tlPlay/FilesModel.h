// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Path.h>
#include <tlCore/ListObserver.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Files model item.
        struct FilesModelItem
        {
            file::Path path;
            file::Path audioPath;

            std::vector<std::string> videoLayers;
            size_t videoLayer = 0;
        };

        //! Files model.
        class FilesModel : public std::enable_shared_from_this<FilesModel>
        {
            TLRENDER_NON_COPYABLE(FilesModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            FilesModel();

        public:
            ~FilesModel();

            //! Create a new files model.
            static std::shared_ptr<FilesModel> create(const std::shared_ptr<system::Context>&);

            //! Observe the files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeFiles() const;

            //! Observe the "A" file.
            std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > > observeA() const;

            //! Observe the "A" file index.
            std::shared_ptr<observer::IValue<int> > observeAIndex() const;

            //! Observe the "B" files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeB() const;

            //! Observe the "B" file indexes.
            std::shared_ptr<observer::IList<int> > observeBIndexes() const;

            //! Observe the active files. The active files are the "A" file
            //! and "B" files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeActive() const;

            //! Add a file.
            void add(const std::shared_ptr<FilesModelItem>&);

            //! Close the current "A" file.
            void close();

            //! Close all the files.
            void closeAll();

            //! Reload all the active files.
            void reload();

            //! Set the "A" file.
            void setA(int index);

            //! Set the "B" files.
            void setB(int index, bool);

            //! Toggle a "B" file.
            void toggleB(int index);

            //! Clear the "B" files.
            void clearB();

            //! Set the "A" file to the first file.
            void first();

            //! Set the "A" file to the last file.
            void last();

            //! Set the "A" file to the next file.
            void next();

            //! Set the "A" file to the previous file.
            void prev();

            //! Set the "B" file to the first file.
            void firstB();

            //! Set the "B" file to the last file.
            void lastB();

            //! Set the "B" file to the next file.
            void nextB();

            //! Set the "B" file to the previous file.
            void prevB();

            //! Observe the layers.
            std::shared_ptr<observer::IList<int> > observeLayers() const;

            //! Set a layer.
            void setLayer(const std::shared_ptr<FilesModelItem>&, int layer);

            //! Set the "A" file to the next layer.
            void nextLayer();

            //! Set the "A" file to the previous layer.
            void prevLayer();

            //! Observe the compare options.
            std::shared_ptr<observer::IValue<timeline::CompareOptions> > observeCompareOptions() const;

            //! Set the compare options.
            void setCompareOptions(const timeline::CompareOptions&);

        private:
            int _index(const std::shared_ptr<FilesModelItem>&) const;
            std::vector<int> _bIndexes() const;
            std::vector<std::shared_ptr<FilesModelItem> > _getActive() const;
            std::vector<int> _getLayers() const;

            TLRENDER_PRIVATE();
        };
    }
}
