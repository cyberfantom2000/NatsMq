#pragma once

#include "Entities.h"

namespace NatsMq
{
    class KeyValueStoreImpl;

    class KeyValueStore
    {
    public:
        KeyValueStore(KeyValueStoreImpl*);

        ~KeyValueStore();

        std::string storeName() const;

        std::vector<std::string> keys() const;

        //! Return byte array with raw bytes
        ByteArray getElement(const std::string& key) const;

        //! Set value or create new key-value element
        void putElement(const std::string& key, const ByteArray& value) const;

        //! Create new key-value element if not exists
        void createElement(const std::string& key, const ByteArray& value) const;

        //! Remove element with history.
        void purgeElement(const std::string& key) const;

        //! Remove element and leave history.
        void removeElement(const std::string& key) const;

        //std::vector<QByteArray> elementHistory(const QByteArray& key) const;

        //KeyValueWatcher createWatcher() const;

    private:
        std::unique_ptr<KeyValueStoreImpl> _impl;
    };

    //    class KeyValueWatcherImpl;

    //    class KeyValueWatcher
    //    {
    //    public:
    //        KeyValueWatcher(KeyValueWatcherImpl*);

    //        void registerCallback();

    //        void start();

    //    private:
    //        std::unique_ptr<KeyValueWatcherImpl> _impl;
    //    };
}
