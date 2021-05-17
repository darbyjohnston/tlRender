// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Observer.h>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace tlr
{
    namespace Observer
    {
        template<typename T, typename U>
        class IMapSubject;

        //! Map observer.
        template<typename T, typename U>
        class Map : public std::enable_shared_from_this<Map<T, U> >
        {
            TLR_NON_COPYABLE(Map);

            void _init(
                const std::weak_ptr<IMapSubject<T, U> >&,
                const std::function<void(const std::map<T, U>&)>&,
                CallbackAction);

            Map();

        public:
            ~Map();

            //! Create a new map observer.
            static std::shared_ptr<Map<T, U> > create(
                const std::weak_ptr<IMapSubject<T, U> >&,
                const std::function<void(const std::map<T, U>&)>&,
                CallbackAction = CallbackAction::Trigger);

            //! Execute the callback.
            void doCallback(const std::map<T, U>&);

        private:
            std::function<void(const std::map<T, U>&)> _callback;
            std::weak_ptr<IMapSubject<T, U> > _subject;
        };

        //! Base class for a map subject.
        template<typename T, typename U>
        class IMapSubject
        {
        public:
            virtual ~IMapSubject() = 0;

            //! Get the map.
            virtual const std::map<T, U>& get() const = 0;

            //! Get the map size.
            virtual std::size_t getSize() const = 0;

            //! Get whether the map is empty.
            virtual bool isEmpty() const = 0;

            //! Does the given key exist?
            virtual bool hasKey(const T&) = 0;

            //! Get a map item.
            virtual const U& getItem(const T&) const = 0;

            //! Get the number of observers.
            std::size_t getObserversCount() const;

        protected:
            void _add(const std::weak_ptr<Map<T, U> >&);
            void _removeExpired();

            std::vector<std::weak_ptr<Map<T, U> > > _observers;

            friend Map<T, U>;
        };

        //! Map subject.
        template<typename T, typename U>
        class MapSubject : public IMapSubject<T, U>
        {
            TLR_NON_COPYABLE(MapSubject);

            MapSubject();
            explicit MapSubject(const std::map<T, U>&);

        public:
            //! Create a new map subject.
            static std::shared_ptr<MapSubject<T, U> > create();

            //! Create a new map subject with the given value.
            static std::shared_ptr<MapSubject<T, U> > create(const std::map<T, U>&);

            //! Set the map.
            void setAlways(const std::map<T, U>&);

            //! Set the map only if it has changed.
            bool setIfChanged(const std::map<T, U>&);

            //! Clear the map.
            void clear();

            //! Set a map item.
            void setItem(const T&, const U&);

            //! Set a map item only if it has changed.
            void setItemOnlyIfChanged(const T&, const U&);

            const std::map<T, U>& get() const override;
            std::size_t getSize() const override;
            bool isEmpty() const override;
            bool hasKey(const T&) override;
            const U& getItem(const T&) const override;

        private:
            std::map<T, U> _value;
        };

    }
}

#include <tlrCore/MapObserverInline.h>
