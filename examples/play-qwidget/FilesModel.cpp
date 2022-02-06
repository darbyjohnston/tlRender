// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FilesModel.h"

#include <tlrCore/StringFormat.h>

#include <QApplication>
#include <QPalette>

namespace tlr
{
    void FilesModel::_init(const std::shared_ptr<core::Context>& context)
    {
        _context = context;

        _files = observer::List<std::shared_ptr<FilesModelItem> >::create();
        _a = observer::Value<std::shared_ptr<FilesModelItem> >::create();
        _b = observer::List<std::shared_ptr<FilesModelItem> >::create();
        _active = observer::List<std::shared_ptr<FilesModelItem> >::create();
        _layers = observer::List<int>::create();
        _imageOptions = observer::List<render::ImageOptions>::create();
        _compareOptions = observer::Value<render::CompareOptions>::create();
    }

    FilesModel::FilesModel()
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
        return _files;
    }

    std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > > FilesModel::observeA() const
    {
        return _a;
    }

    std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > FilesModel::observeB() const
    {
        return _b;
    }

    std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > FilesModel::observeActive() const
    {
        return _active;
    }

    void FilesModel::add(const std::shared_ptr<FilesModelItem>& item)
    {
        _files->pushBack(item);

        _a->setIfChanged(_files->getItem(_files->getSize() - 1));

        _active->setIfChanged(_getActive());
        _layers->setIfChanged(_getLayers());
        _imageOptions->setIfChanged(_getImageOptions());
    }

    void FilesModel::close()
    {
        if (_a->get())
        {
            auto files = _files->get();
            const auto i = std::find(files.begin(), files.end(), _a->get());
            if (i != files.end())
            {
                const int aPrevIndex = _index(_a->get());

                files.erase(i);
                _files->setIfChanged(files);

                const int aNewIndex = math::clamp(aPrevIndex, 0, static_cast<int>(files.size()) - 1);
                _a->setIfChanged(aNewIndex != -1 ? files[aNewIndex] : nullptr);

                auto b = _b->get();
                auto j = b.begin();
                while (j != b.end())
                {
                    const auto k = std::find(files.begin(), files.end(), *j);
                    if (k == files.end())
                    {
                        b.erase(j);
                    }
                    else
                    {
                        ++j;
                    }
                }
                _b->setIfChanged(b);

                _active->setIfChanged(_getActive());
                _layers->setIfChanged(_getLayers());
                _imageOptions->setIfChanged(_getImageOptions());
            }
        }
    }

    void FilesModel::closeAll()
    {
        _files->clear();

        _a->setIfChanged(nullptr);

        _b->clear();

        _active->setIfChanged(_getActive());
        _layers->setIfChanged(_getLayers());
        _imageOptions->setIfChanged(_getImageOptions());
    }

    void FilesModel::setA(int index)
    {
        const int prevIndex = _index(_a->get());
        if (index >= 0 && index < _files->getSize() && index != prevIndex)
        {
            _a->setIfChanged(_files->getItem(index));

            _active->setIfChanged(_getActive());
            _layers->setIfChanged(_getLayers());
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    void FilesModel::setB(int index, bool value)
    {
        if (index >= 0 && index < _files->getSize())
        {
            auto b = _b->get();
            int removedIndex = -1;
            const auto bIndexes = _bIndexes();
            const auto i = std::find(bIndexes.begin(), bIndexes.end(), index);
            if (value && i == bIndexes.end())
            {
                b.push_back(_files->getItem(index));
                switch (_compareOptions->get().mode)
                {
                case render::CompareMode::A:
                case render::CompareMode::B:
                case render::CompareMode::Horizontal:
                case render::CompareMode::Vertical:
                case render::CompareMode::Free:
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
            _b->setIfChanged(b);

            _active->setIfChanged(_getActive());
            _layers->setIfChanged(_getLayers());
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    void FilesModel::toggleB(int index)
    {
        if (index >= 0 && index < _files->getSize())
        {
            const auto& item = _files->getItem(index);
            setB(index, _b->indexOf(item) == observer::invalidListIndex);
        }
    }

    void FilesModel::first()
    {
        const int prevIndex = _index(_a->get());
        if (!_files->isEmpty() && prevIndex != 0)
        {
            _a->setIfChanged(_files->getItem(0));

            _active->setIfChanged(_getActive());
            _layers->setIfChanged(_getLayers());
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    void FilesModel::last()
    {
        const int index = static_cast<int>(_files->getSize()) - 1;
        const int prevIndex = _index(_a->get());
        if (!_files->isEmpty() && index != prevIndex)
        {
            _a->setIfChanged(_files->getItem(index));

            _active->setIfChanged(_getActive());
            _layers->setIfChanged(_getLayers());
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    void FilesModel::next()
    {
        if (!_files->isEmpty())
        {
            const int prevIndex = _index(_a->get());
            int index = prevIndex + 1;
            if (index >= _files->getSize())
            {
                index = 0;
            }
            _a->setIfChanged(_files->getItem(index));

            _active->setIfChanged(_getActive());
            _layers->setIfChanged(_getLayers());
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    void FilesModel::prev()
    {
        if (!_files->isEmpty())
        {
            const int prevIndex = _index(_a->get());
            int index = prevIndex - 1;
            if (index < 0)
            {
                index = _files->getSize() - 1;
            }
            _a->setIfChanged(_files->getItem(index));

            _active->setIfChanged(_getActive());
            _layers->setIfChanged(_getLayers());
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    std::shared_ptr<observer::IList<int> > FilesModel::observeLayers() const
    {
        return _layers;
    }

    void FilesModel::setLayer(const std::shared_ptr<FilesModelItem>& item, int layer)
    {
        const int index = _index(item);
        if (index != -1 &&
            layer < _files->getItem(index)->avInfo.video.size() &&
            layer != _files->getItem(index)->videoLayer)
        {
            _files->getItem(index)->videoLayer = layer;
            _layers->setIfChanged(_getLayers());
        }
    }

    void FilesModel::nextLayer()
    {
        const int index = _index(_a->get());
        if (index != -1)
        {
            auto item = _files->getItem(index);
            int layer = item->videoLayer + 1;
            if (layer >= item->avInfo.video.size())
            {
                layer = 0;
            }
            item->videoLayer = layer;
            _layers->setIfChanged(_getLayers());
        }
    }

    void FilesModel::prevLayer()
    {
        const int index = _index(_a->get());
        if (index != -1)
        {
            auto item = _files->getItem(index);
            int layer = item->videoLayer - 1;
            if (layer < 0)
            {
                layer = static_cast<int>(item->avInfo.video.size()) - 1;
            }
            item->videoLayer = std::max(layer, 0);
            _layers->setIfChanged(_getLayers());
        }
    }

    std::shared_ptr<observer::IList<render::ImageOptions> > FilesModel::observeImageOptions() const
    {
        return _imageOptions;
    }

    void FilesModel::setImageOptions(const render::ImageOptions& imageOptions)
    {
        const int index = _index(_a->get());
        if (index != -1 &&
            imageOptions != _files->getItem(index)->imageOptions)
        {
            _files->getItem(index)->imageOptions = imageOptions;
            _imageOptions->setIfChanged(_getImageOptions());
        }
    }

    std::shared_ptr<observer::IValue<render::CompareOptions> > FilesModel::observeCompareOptions() const
    {
        return _compareOptions;
    }

    void FilesModel::setCompareOptions(const render::CompareOptions& value)
    {
        if (_compareOptions->setIfChanged(value))
        {
            switch (_compareOptions->get().mode)
            {
            case render::CompareMode::A:
            case render::CompareMode::B:
            case render::CompareMode::Horizontal:
            case render::CompareMode::Vertical:
            case render::CompareMode::Free:
            {
                auto b = _b->get();
                while (b.size() > 1)
                {
                    b.pop_back();
                }
                if (_b->setIfChanged(b))
                {
                    _active->setIfChanged(_getActive());
                    _layers->setIfChanged(_getLayers());
                    _imageOptions->setIfChanged(_getImageOptions());
                }
                break;
            }
            default: break;
            }
        }
    }

    int FilesModel::_index(const std::shared_ptr<FilesModelItem>& item) const
    {
        size_t index = _files->indexOf(item);
        return index != observer::invalidListIndex ? index : -1;
    }

    std::vector<int> FilesModel::_bIndexes() const
    {
        std::vector<int> out;
        for (const auto& b : _b->get())
        {
            out.push_back(_index(b));
        }
        return out;
    }

    std::vector<std::shared_ptr<FilesModelItem> > FilesModel::_getActive() const
    {
        std::vector<std::shared_ptr<FilesModelItem> > out;
        if (_a->get())
        {
            out.push_back(_a->get());
        }
        for (const auto& b : _b->get())
        {
            out.push_back(b);
        }
        return out;
    }

    std::vector<int> FilesModel::_getLayers() const
    {
        std::vector<int> out;
        if (_a->get())
        {
            out.push_back(_a->get()->videoLayer);
        }
        for (const auto& b : _b->get())
        {
            out.push_back(b->videoLayer);
        }
        return out;
    }

    std::vector<render::ImageOptions> FilesModel::_getImageOptions() const
    {
        std::vector<tlr::render::ImageOptions> out;
        if (_a->get())
        {
            out.push_back(_a->get()->imageOptions);
        }
        for (const auto& b : _b->get())
        {
            out.push_back(b->imageOptions);
        }
        return out;
    }

    FilesItemModel::FilesItemModel(
        const std::shared_ptr<FilesModel>& filesModel,
        const std::shared_ptr<core::Context>& context,
        QObject* parent) :
        QAbstractTableModel(parent),
        _context(context),
        _filesModel(filesModel)
    {
        _filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
            filesModel->observeFiles(),
            [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
            {
                beginResetModel();
                _files = value;
                for (auto i : _files)
                {
                    auto j = _thumbnailProviders.find(i);
                    if (j == _thumbnailProviders.end())
                    {
                        if (auto context = _context.lock())
                        {
                            try
                            {
                                auto timeline = timeline::Timeline::create(i->path.get(), context);
                                _thumbnailProviders[i] = new qt::TimelineThumbnailProvider(timeline, context);
                                connect(
                                    _thumbnailProviders[i],
                                    SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                                    SLOT(_thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
                                _thumbnailProviders[i]->request(timeline->getGlobalStartTime(), QSize(120, 80));
                            }
                            catch (const std::exception&)
                            {}
                        }
                    }
                }
                endResetModel();
            });
        _activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
            filesModel->observeActive(),
            [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
            {
                _active = value;
            });
        _layersObserver = observer::ListObserver<int>::create(
            filesModel->observeLayers(),
            [this](const std::vector<int>& value)
            {
                for (size_t i = 0; i < value.size() && i < _active.size(); ++i)
                {
                    const auto j = std::find(_files.begin(), _files.end(), _active[i]);
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

    const std::vector<std::shared_ptr<FilesModelItem> >& FilesItemModel::files() const
    {
        return _files;
    }

    int FilesItemModel::rowCount(const QModelIndex&) const
    {
        return _files.size();
    }

    int FilesItemModel::columnCount(const QModelIndex& parent) const
    {
        return 2;
    }

    Qt::ItemFlags FilesItemModel::flags(const QModelIndex& index) const
    {
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

    QVariant FilesItemModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _files.size() &&
            index.column() >= 0 &&
            index.column() < 4)
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
                    const auto i = _thumbnails.find(item);
                    if (i != _thumbnails.end())
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

    bool FilesItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        bool out = false;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _files.size() &&
            index.column() >= 0 &&
            index.column() < 4)
        {
            const auto& item = _files[index.row()];
            switch (role)
            {
            case Qt::EditRole:
                switch (index.column())
                {
                case 1:
                    _filesModel->setLayer(item, value.toInt());
                    out = true;
                    break;
                }
                break;
            default: break;
            }
        }
        return out;
    }

    QVariant FilesItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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

    void FilesItemModel::_thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >& value)
    {
        if (!value.isEmpty())
        {
            for (auto i = _thumbnailProviders.begin(); i != _thumbnailProviders.end(); ++i)
            {
                if (i->second == sender())
                {
                    _thumbnails[i->first] = value[0].second;
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
                    _thumbnailProviders.erase(i);
                    break;
                }
            }
        }
    }

    int FilesItemModel::_index(const std::shared_ptr<FilesModelItem>& item) const
    {
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

    FilesAModel::FilesAModel(
        const std::shared_ptr<FilesModel>& filesModel,
        const std::shared_ptr<core::Context>& context,
        QObject* parent) :
        FilesItemModel(filesModel, context, parent)
    {
        _aObserver = observer::ValueObserver<std::shared_ptr<FilesModelItem> >::create(
            filesModel->observeA(),
            [this](const std::shared_ptr<FilesModelItem>& value)
            {
                const int prevIndex = _index(_a);
                _a = value;
                const int index = _index(_a);
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

    QVariant FilesAModel::data(const QModelIndex& index, int role) const
    {
        QVariant out = FilesItemModel::data(index, role);
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _files.size() &&
            index.column() >= 0 &&
            index.column() < 4)
        {
            const auto& item = _files[index.row()];
            switch (role)
            {
            case Qt::BackgroundRole:
            {
                const int aIndex = _index(_a);
                if (aIndex == index.row())
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                }
                break;
            }
            case Qt::ForegroundRole:
            {
                const int aIndex = _index(_a);
                if (aIndex == index.row())
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                }
                break;
            }
            default: break;
            }
        }
        return out;
    }

    FilesBModel::FilesBModel(
        const std::shared_ptr<FilesModel>& filesModel,
        const std::shared_ptr<core::Context>& context,
        QObject* parent) :
        FilesItemModel(filesModel, context, parent)
    {
        _bObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
            filesModel->observeB(),
            [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
            {
                const auto prevIndexes = _bIndexes();
                _b = value;
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

    QVariant FilesBModel::data(const QModelIndex& index, int role) const
    {
        QVariant out = FilesItemModel::data(index, role);
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _files.size() &&
            index.column() >= 0 &&
            index.column() < 4)
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
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                }
                break;
            }
            case Qt::ForegroundRole:
            {
                const auto bIndexes = _bIndexes();
                const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                if (i != bIndexes.end())
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
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
        std::vector<int> out;
        for (const auto& b : _b)
        {
            out.push_back(_index(b));
        }
        return out;
    }
}
