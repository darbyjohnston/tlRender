// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/FilesModel.h>

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlCore/StringFormat.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play
    {
        struct FilesModel::Private
        {
            std::weak_ptr<core::Context> context;
            std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > > files;
            std::shared_ptr<observer::Value<std::shared_ptr<FilesModelItem> > > a;
            std::shared_ptr<observer::Value<int> > aIndex;
            std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > > b;
            std::shared_ptr<observer::List<int> > bIndexes;
            std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > > active;
            std::shared_ptr<observer::List<int> > layers;
            std::shared_ptr<observer::Value<render::CompareOptions> > compareOptions;
        };

        void FilesModel::_init(const std::shared_ptr<core::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.files = observer::List<std::shared_ptr<FilesModelItem> >::create();
            p.a = observer::Value<std::shared_ptr<FilesModelItem> >::create();
            p.aIndex = observer::Value<int>::create();
            p.b = observer::List<std::shared_ptr<FilesModelItem> >::create();
            p.bIndexes = observer::List<int>::create();
            p.active = observer::List<std::shared_ptr<FilesModelItem> >::create();
            p.layers = observer::List<int>::create();
            p.compareOptions = observer::Value<render::CompareOptions>::create();
        }

        FilesModel::FilesModel() :
            _p(new Private)
        {}

        FilesModel::~FilesModel()
        {}

        std::shared_ptr<FilesModel> FilesModel::create(const std::shared_ptr<core::Context>& context)
        {
            auto out = std::shared_ptr<FilesModel>(new FilesModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > FilesModel::observeFiles() const
        {
            return _p->files;
        }

        std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > > FilesModel::observeA() const
        {
            return _p->a;
        }

        std::shared_ptr<observer::IValue<int> > FilesModel::observeAIndex() const
        {
            return _p->aIndex;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > FilesModel::observeB() const
        {
            return _p->b;
        }

        std::shared_ptr<observer::IList<int> > FilesModel::observeBIndexes() const
        {
            return _p->bIndexes;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > FilesModel::observeActive() const
        {
            return _p->active;
        }

        void FilesModel::add(const std::shared_ptr<FilesModelItem>& item)
        {
            TLRENDER_P();

            p.files->pushBack(item);

            p.a->setIfChanged(p.files->getItem(p.files->getSize() - 1));
            p.aIndex->setIfChanged(_index(p.a->get()));

            if (p.b->isEmpty())
            {
                p.b->pushBack(p.a->get());
                p.bIndexes->setIfChanged(_bIndexes());
            }

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::close()
        {
            TLRENDER_P();
            if (p.a->get())
            {
                auto files = p.files->get();
                const auto i = std::find(files.begin(), files.end(), p.a->get());
                if (i != files.end())
                {
                    const int aPrevIndex = _index(p.a->get());

                    files.erase(i);
                    p.files->setIfChanged(files);

                    const int aNewIndex = math::clamp(aPrevIndex, 0, static_cast<int>(files.size()) - 1);
                    p.a->setIfChanged(aNewIndex != -1 ? files[aNewIndex] : nullptr);
                    p.aIndex->setIfChanged(_index(p.a->get()));

                    auto b = p.b->get();
                    auto j = b.begin();
                    while (j != b.end())
                    {
                        const auto k = std::find(files.begin(), files.end(), *j);
                        if (k == files.end())
                        {
                            j = b.erase(j);
                        }
                        else
                        {
                            ++j;
                        }
                    }
                    p.b->setIfChanged(b);
                    p.bIndexes->setIfChanged(_bIndexes());

                    p.active->setIfChanged(_getActive());
                    p.layers->setIfChanged(_getLayers());
                }
            }
        }

        void FilesModel::closeAll()
        {
            TLRENDER_P();

            p.files->clear();

            p.a->setIfChanged(nullptr);
            p.aIndex->setIfChanged(-1);

            p.b->clear();
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::setA(int index)
        {
            TLRENDER_P();
            const int prevIndex = _index(p.a->get());
            if (index >= 0 && index < p.files->getSize() && index != prevIndex)
            {
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::setB(int index, bool value)
        {
            TLRENDER_P();
            if (index >= 0 && index < p.files->getSize())
            {
                auto b = p.b->get();
                int removedIndex = -1;
                const auto bIndexes = _bIndexes();
                const auto i = std::find(bIndexes.begin(), bIndexes.end(), index);
                if (value && i == bIndexes.end())
                {
                    b.push_back(p.files->getItem(index));
                    switch (p.compareOptions->get().mode)
                    {
                    case render::CompareMode::A:
                    case render::CompareMode::B:
                    case render::CompareMode::Wipe:
                        if (b.size() > 1)
                        {
                            removedIndex = _index(b.front());
                            b.erase(b.begin());
                        }
                        break;
                    default: break;
                    }
                }
                else if (!value && i != bIndexes.end())
                {
                    b.erase(b.begin() + (i - bIndexes.begin()));
                }
                p.b->setIfChanged(b);
                p.bIndexes->setIfChanged(_bIndexes());

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::toggleB(int index)
        {
            TLRENDER_P();
            if (index >= 0 && index < p.files->getSize())
            {
                const auto& item = p.files->getItem(index);
                setB(index, p.b->indexOf(item) == observer::invalidListIndex);
            }
        }

        void FilesModel::clearB()
        {
            TLRENDER_P();
            if (!p.b->isEmpty())
            {
                p.b->clear();
                p.bIndexes->setIfChanged(_bIndexes());

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::first()
        {
            TLRENDER_P();
            const int prevIndex = _index(p.a->get());
            if (!p.files->isEmpty() && prevIndex != 0)
            {
                p.a->setIfChanged(p.files->getItem(0));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::last()
        {
            TLRENDER_P();
            const int index = static_cast<int>(p.files->getSize()) - 1;
            const int prevIndex = _index(p.a->get());
            if (!p.files->isEmpty() && index != prevIndex)
            {
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::next()
        {
            TLRENDER_P();
            if (!p.files->isEmpty())
            {
                const int prevIndex = _index(p.a->get());
                int index = prevIndex + 1;
                if (index >= p.files->getSize())
                {
                    index = 0;
                }
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::prev()
        {
            TLRENDER_P();
            if (!p.files->isEmpty())
            {
                const int prevIndex = _index(p.a->get());
                int index = prevIndex - 1;
                if (index < 0)
                {
                    index = p.files->getSize() - 1;
                }
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::firstB()
        {
            TLRENDER_P();

            p.b->clear();
            if (!p.files->isEmpty())
            {
                p.b->pushBack(p.files->getItem(0));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::lastB()
        {
            TLRENDER_P();

            p.b->clear();
            if (!p.files->isEmpty())
            {
                p.b->pushBack(p.files->getItem(p.files->getSize() - 1));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::nextB()
        {
            TLRENDER_P();

            int index = 0;
            const auto bIndexes = _bIndexes();
            if (!bIndexes.empty())
            {
                index = bIndexes[bIndexes.size() - 1];
                ++index;
            }
            if (index >= p.files->getSize())
            {
                index = 0;
            }
            p.b->clear();
            if (index >= 0 && index <= p.files->getSize())
            {
                p.b->pushBack(p.files->getItem(index));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::prevB()
        {
            TLRENDER_P();

            int index = 0;
            const auto bIndexes = _bIndexes();
            if (!bIndexes.empty())
            {
                index = bIndexes[0];
                --index;
            }
            if (index < 0)
            {
                index = static_cast<int>(p.files->getSize()) - 1;
            }
            p.b->clear();
            if (index >= 0 && index <= p.files->getSize())
            {
                p.b->pushBack(p.files->getItem(index));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        std::shared_ptr<observer::IList<int> > FilesModel::observeLayers() const
        {
            return _p->layers;
        }

        void FilesModel::setLayer(const std::shared_ptr<FilesModelItem>& item, int layer)
        {
            TLRENDER_P();
            const int index = _index(item);
            if (index != -1 &&
                layer < p.files->getItem(index)->avInfo.video.size() &&
                layer != p.files->getItem(index)->videoLayer)
            {
                p.files->getItem(index)->videoLayer = layer;
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::nextLayer()
        {
            TLRENDER_P();
            const int index = _index(p.a->get());
            if (index != -1)
            {
                auto item = p.files->getItem(index);
                int layer = item->videoLayer + 1;
                if (layer >= item->avInfo.video.size())
                {
                    layer = 0;
                }
                item->videoLayer = layer;
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::prevLayer()
        {
            TLRENDER_P();
            const int index = _index(p.a->get());
            if (index != -1)
            {
                auto item = p.files->getItem(index);
                int layer = item->videoLayer - 1;
                if (layer < 0)
                {
                    layer = static_cast<int>(item->avInfo.video.size()) - 1;
                }
                item->videoLayer = std::max(layer, 0);
                p.layers->setIfChanged(_getLayers());
            }
        }

        std::shared_ptr<observer::IValue<render::CompareOptions> > FilesModel::observeCompareOptions() const
        {
            return _p->compareOptions;
        }

        void FilesModel::setCompareOptions(const render::CompareOptions& value)
        {
            TLRENDER_P();
            if (p.compareOptions->setIfChanged(value))
            {
                switch (p.compareOptions->get().mode)
                {
                case render::CompareMode::A:
                case render::CompareMode::B:
                case render::CompareMode::Wipe:
                {
                    auto b = p.b->get();
                    while (b.size() > 1)
                    {
                        b.pop_back();
                    }
                    if (p.b->setIfChanged(b))
                    {
                        p.bIndexes->setIfChanged(_bIndexes());
                    }
                    break;
                }
                default: break;
                }

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        int FilesModel::_index(const std::shared_ptr<FilesModelItem>& item) const
        {
            TLRENDER_P();
            size_t index = p.files->indexOf(item);
            return index != observer::invalidListIndex ? index : -1;
        }

        std::vector<int> FilesModel::_bIndexes() const
        {
            TLRENDER_P();
            std::vector<int> out;
            for (const auto& b : p.b->get())
            {
                out.push_back(_index(b));
            }
            return out;
        }

        std::vector<std::shared_ptr<FilesModelItem> > FilesModel::_getActive() const
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<FilesModelItem> > out;
            if (p.a->get())
            {
                out.push_back(p.a->get());
            }
            switch (p.compareOptions->get().mode)
            {
            case render::CompareMode::B:
            case render::CompareMode::Wipe:
            case render::CompareMode::Tile:
                for (const auto& b : p.b->get())
                {
                    out.push_back(b);
                }
                break;
            default: break;
            }
            return out;
        }

        std::vector<int> FilesModel::_getLayers() const
        {
            TLRENDER_P();
            std::vector<int> out;
            if (p.a->get())
            {
                out.push_back(p.a->get()->videoLayer);
            }
            switch (p.compareOptions->get().mode)
            {
            case render::CompareMode::B:
            case render::CompareMode::Wipe:
            case render::CompareMode::Tile:
                for (const auto& b : p.b->get())
                {
                    out.push_back(b->videoLayer);
                }
                break;
            default: break;
            }
            return out;
        }

        struct FilesTableModel::Private
        {
            std::weak_ptr<core::Context> context;
            std::shared_ptr<FilesModel> filesModel;
            std::vector<std::shared_ptr<FilesModelItem> > active;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::map<std::shared_ptr<FilesModelItem>, QImage> thumbnails;
            std::map<std::shared_ptr<FilesModelItem>, qt::TimelineThumbnailProvider*> thumbnailProviders;
        };

        FilesTableModel::FilesTableModel(
            const std::shared_ptr<FilesModel>& filesModel,
            const std::shared_ptr<core::Context>& context,
            QObject* parent) :
            QAbstractTableModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;
            p.filesModel = filesModel;

            p.filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    beginResetModel();
                    _files = value;
                    for (auto i : _files)
                    {
                        auto j = _p->thumbnailProviders.find(i);
                        if (j == _p->thumbnailProviders.end())
                        {
                            if (auto context = _p->context.lock())
                            {
                                try
                                {
                                    auto timeline = timeline::Timeline::create(i->path.get(), context);
                                    _p->thumbnailProviders[i] = new qt::TimelineThumbnailProvider(timeline, context);
                                    connect(
                                        _p->thumbnailProviders[i],
                                        SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                                        SLOT(_thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
                                    _p->thumbnailProviders[i]->request(timeline->getGlobalStartTime(), QSize(120, 80));
                                }
                                catch (const std::exception&)
                                {
                                }
                            }
                        }
                    }
                    endResetModel();
                });
            p.activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _p->active = value;
                });
            p.layersObserver = observer::ListObserver<int>::create(
                filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _p->active.size(); ++i)
                    {
                        const auto j = std::find(_files.begin(), _files.end(), _p->active[i]);
                        if (j != _files.end())
                        {
                            const int index = j - _files.begin();
                            Q_EMIT dataChanged(
                                this->index(index, 1),
                                this->index(index, 1),
                                { Qt::DisplayRole, Qt::EditRole });
                        }
                    }
                });
        }

        FilesTableModel::~FilesTableModel()
        {}

        const std::vector<std::shared_ptr<FilesModelItem> >& FilesTableModel::files() const
        {
            return _files;
        }

        int FilesTableModel::rowCount(const QModelIndex&) const
        {
            return _files.size();
        }

        int FilesTableModel::columnCount(const QModelIndex& parent) const
        {
            return 2;
        }

        Qt::ItemFlags FilesTableModel::flags(const QModelIndex& index) const
        {
            TLRENDER_P();
            Qt::ItemFlags out = Qt::NoItemFlags;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                out |= Qt::ItemIsEnabled;
                out |= Qt::ItemIsSelectable;
                switch (index.column())
                {
                case 1: out |= Qt::ItemIsEditable; break;
                }
            }
            return out;
        }

        QVariant FilesTableModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    std::string s;
                    switch (index.column())
                    {
                    case 0:
                        s = item->path.get(-1, false);
                        break;
                    case 1:
                        if (!item->avInfo.video.empty() &&
                            item->videoLayer < item->avInfo.video.size())
                        {
                            s = item->avInfo.video[item->videoLayer].name;
                        }
                        break;
                    }
                    out.setValue(QString::fromUtf8(s.c_str()));
                    break;
                }
                case Qt::DecorationRole:
                    switch (index.column())
                    {
                    case 0:
                    {
                        const auto i = p.thumbnails.find(item);
                        if (i != p.thumbnails.end())
                        {
                            out.setValue(i->second);
                        }
                        break;
                    }
                    }
                    break;
                case Qt::EditRole:
                    switch (index.column())
                    {
                    case 1: out.setValue(item->videoLayer); break;
                    }
                    break;
                case Qt::ToolTipRole:
                    out.setValue(QString::fromUtf8(item->path.get().c_str()));
                    break;
                default: break;
                }
            }
            return out;
        }

        bool FilesTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
        {
            TLRENDER_P();
            bool out = false;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::EditRole:
                    switch (index.column())
                    {
                    case 1:
                        p.filesModel->setLayer(item, value.toInt());
                        out = true;
                        break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        QVariant FilesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
        {
            QVariant out;
            if (Qt::Horizontal == orientation)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    switch (section)
                    {
                    case 0: out = tr("Name"); break;
                    case 1: out = tr("Layer"); break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        void FilesTableModel::_thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >& value)
        {
            TLRENDER_P();
            if (!value.isEmpty())
            {
                for (auto i = p.thumbnailProviders.begin(); i != p.thumbnailProviders.end(); ++i)
                {
                    if (i->second == sender())
                    {
                        p.thumbnails[i->first] = value[0].second;
                        const auto j = std::find(_files.begin(), _files.end(), i->first);
                        if (j != _files.end())
                        {
                            const int index = j - _files.begin();
                            Q_EMIT dataChanged(
                                this->index(index, 0),
                                this->index(index, 0),
                                { Qt::DecorationRole });
                        }
                        delete i->second;
                        p.thumbnailProviders.erase(i);
                        break;
                    }
                }
            }
        }

        int FilesTableModel::_index(const std::shared_ptr<FilesModelItem>& item) const
        {
            TLRENDER_P();
            int out = -1;
            if (!_files.empty())
            {
                const auto i = std::find(_files.begin(), _files.end(), item);
                if (i != _files.end())
                {
                    out = i - _files.begin();
                }
            }
            return out;
        }

        struct FilesAModel::Private
        {
            std::shared_ptr<FilesModelItem> a;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<FilesModelItem> > > aObserver;
        };

        FilesAModel::FilesAModel(
            const std::shared_ptr<FilesModel>& filesModel,
            const std::shared_ptr<core::Context>& context,
            QObject* parent) :
            FilesTableModel(filesModel, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.aObserver = observer::ValueObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeA(),
                [this](const std::shared_ptr<FilesModelItem>& value)
                {
                    const int prevIndex = _index(_p->a);
                    _p->a = value;
                    const int index = _index(_p->a);
                    Q_EMIT dataChanged(
                        this->index(index, 0),
                        this->index(index, 1),
                        { Qt::BackgroundRole, Qt::ForegroundRole });
                    Q_EMIT dataChanged(
                        this->index(prevIndex, 0),
                        this->index(prevIndex, 1),
                        { Qt::BackgroundRole, Qt::ForegroundRole });
                });
        }

        FilesAModel::~FilesAModel()
        {}

        QVariant FilesAModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out = FilesTableModel::data(index, role);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::BackgroundRole:
                {
                    const int aIndex = _index(p.a);
                    if (aIndex == index.row())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                }
                case Qt::ForegroundRole:
                {
                    const int aIndex = _index(p.a);
                    if (aIndex == index.row())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                }
                default: break;
                }
            }
            return out;
        }

        struct FilesBModel::Private
        {
            std::vector<std::shared_ptr<FilesModelItem> > b;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > bObserver;
        };

        FilesBModel::FilesBModel(
            const std::shared_ptr<FilesModel>& filesModel,
            const std::shared_ptr<core::Context>& context,
            QObject* parent) :
            FilesTableModel(filesModel, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.bObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeB(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    const auto prevIndexes = _bIndexes();
                    _p->b = value;
                    for (const auto& i : _bIndexes())
                    {
                        Q_EMIT dataChanged(
                            this->index(i, 0),
                            this->index(i, 1),
                            { Qt::BackgroundRole, Qt::ForegroundRole });
                    }
                    for (const auto& i : prevIndexes)
                    {
                        Q_EMIT dataChanged(
                            this->index(i, 0),
                            this->index(i, 1),
                            { Qt::BackgroundRole, Qt::ForegroundRole });
                    }
                });
        }

        FilesBModel::~FilesBModel()
        {}

        QVariant FilesBModel::data(const QModelIndex& index, int role) const
        {
            QVariant out = FilesTableModel::data(index, role);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::BackgroundRole:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    if (i != bIndexes.end())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                }
                case Qt::ForegroundRole:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    if (i != bIndexes.end())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                }
                default: break;
                }
            }
            return out;
        }

        std::vector<int> FilesBModel::_bIndexes() const
        {
            TLRENDER_P();
            std::vector<int> out;
            for (const auto& b : p.b)
            {
                out.push_back(_index(b));
            }
            return out;
        }
    }
}
