#include <functional>
#include <utility>
#include <vector>
#include <list>
#include <algorithm>

template <class Key, class Value>
class HashTable {
public:
    typedef Key                  key_type;
    typedef Value                value_type;
    typedef std::pair<Key,Value> item_type;
    typedef std::list<item_type> bucket_type;
    typedef std::size_t          hash_type;

    template<bool Const>
    class iterator_base {
    public:
        typedef typename std::conditional<Const,const HashTable*,HashTable*>::type
            table_pointer;
        typedef typename std::conditional<Const,const item_type&,item_type&>::type
            reference_type;
        typedef typename std::conditional<Const,const item_type*,item_type*>::type
            pointer_type;
        typedef typename std::conditional<Const,typename bucket_type::const_iterator,typename bucket_type::iterator>::type
            iterator_type;
        typedef typename std::conditional<Const,const bucket_type &,bucket_type&>::type
            bucket_reference;

        iterator_base() : table(nullptr), bucket_index(-1) {}

        /**
         * \brief Move the iterator to the next element
         */
        iterator_base& operator++() {
            if (table && bucket_index >= 0 && bucket_index < table->bucket_count()) {
                ++iterator;
            }
            normalize();
            return *this;
        }

        /**
         * \brief Move the iterator to the previous element
         */
        iterator_base& operator--() {
            if (table) {
                if (bucket_index == -1) {
                    bucket_index = table->bucket_count() - 1;
                    iterator = bucket().end();
                }

                if (iterator == bucket().begin()) {
                    while (bucket_index >= 0 && bucket().empty()) {
                        --bucket_index;
                    }

                    if (bucket_index >= 0) {
                        iterator = bucket().end();
                        --iterator;
                    }
                } else {
                    --iterator;
                }
            }
            return *this;
        }
        iterator_base operator++ (int) {
            iterator_base copy = *this;
            ++*this;
            return copy;
        }
        iterator_base operator-- (int) {
            iterator_base copy = *this;
            --this;
            return copy;
        }

        /**
         * \brief Get reference to the pair
         */
        reference_type operator* () const {
            return *operator->();
        }

        /**
         * \brief Get pointer to the pair
         */
        pointer_type operator-> () const {
            if (table && bucket_index >= 0 && iterator != bucket().end())
                return &*iterator;
            return nullptr;
        }

        bool operator== (const iterator_base& other) const {
            return table == other.table && bucket_index == other.bucket_index &&
                (bucket_index == -1 || iterator == other.iterator);
        }

        bool operator!= (const iterator_base& other) const {
            return ! (*this == other);
        }

    private:
        iterator_base(table_pointer table, int bucket_index, iterator_type iterator)
            : table(table), bucket_index(bucket_index), iterator(iterator) {
            normalize();
        }

        /**
         * \brief Get reference to the pointed bucket
         */
        bucket_reference bucket() const {
            return table->bucket_list[bucket_index];
        }

        /**
         * \brief If the iterator is pointing to the end of a bucket, move it
         *  forward until we find a non-empty element
         */
        void normalize() {
            if (!table || bucket_index < 0 || bucket_index >= table->bucket_count()) {
                bucket_index = -1;
                return;
            }
            if (iterator == bucket().end()) {
                bucket_index++;
                if (bucket_index < table->bucket_count())
                    iterator =  bucket().begin();
                normalize();
            }
        }

        table_pointer table;
        int           bucket_index;
        iterator_type iterator;

        friend class HashTable;
    };
    typedef iterator_base<false> iterator;
    typedef iterator_base<true>  const_iterator;

    explicit HashTable(int buckets=16) : bucket_list(std::max(1,buckets)) {}

    /**
     * \brief Insert a new item or assign if the key exists
     * \complexity Best: O(1) Worst: O(n) Always: O(m) where m is the # of items in the bucket
     */
    iterator insert_or_assign (const Key& key, const Value& value) {
        hash_type hash = KeyToBucket(key);
        bucket_type& bucket = bucket_list[hash];
        auto iter = find_key(bucket, key);
        if (iter != bucket.end()) {
            *iter = value;
            return iterator(this, hash, iter);
        } else {
            bucket.push_front(std::make_pair(key, value));
            size_++;
            return iterator(this, hash, bucket.begin());
        }
    }

    /**
     * \brief Insert a new item (might cause duplicate entries)
     * \complexity O(1)
     */
    iterator insert (const Key& key, const Value& value) {
        hash_type hash = KeyToBucket(key);
        bucket_type& bucket = bucket_list[hash];
        bucket.push_front(std::make_pair(key, value));
        size_++;
        return iterator(this, hash, bucket.begin());
    }

    /**
     * \brief Erase an existing item (the first one if multiple items with the same key exist)
     * \complexity Best: O(1) Worst: O(n) Always: O(m) where m is the # of items in the bucket
     */
    iterator erase (const Key& key) {
        hash_type hash = KeyToBucket(key);
        bucket_type& bucket = bucket_list[hash];
        auto iter = find_key(bucket, key);
        if (iter == bucket.end())
            return end();
        size_--;
        return iterator(this, hash, bucket.erase(iter));
    }

    /**
     * \brief Erase by iterator
     */
    iterator erase (iterator it) {
        if (it == end() || it.table != this)
            return end();
        it.iterator = it.bucket().erase(it.iterator);
        if (it.iterator != it.bucket().end())
            size_--;
        it.normalize();
        return it;
    }

    /**
     * \brief Find an element
     * \complexity Best: O(1) Worst: O(n) Always: O(m) where m is the # of items in the bucket
     */
    iterator find(const Key& key) {
        hash_type hash = KeyToBucket(key);
        bucket_type& bucket = bucket_list[hash];
        auto iter = find_key(bucket, key);
        if (iter == bucket.end())
            return end();
        return iterator(this, hash, iter);
    }

    /**
     * \brief Get reference to given value (inserted with default-constructed value if not present)
     * \complexity Best: O(1) Worst: O(n) Always: O(m) where m is the # of items in the bucket
     */
    value_type& operator[] (const Key& key) {
        hash_type hash = KeyToBucket(key);
        bucket_type& bucket = bucket_list[hash];
        auto iter = find_key(bucket, key);
        if (iter == bucket.end()) {
            bucket.push_front(std::make_pair(key, value_type()));
            size_++;
            return bucket.front().second;
        }
        return iter->second;
    }

    /**
     * \brief Print the tree tructure to stdout
     * \complexity O(n)
     */
    void print_structure() const {
        for (hash_type i = 0; i < bucket_list.size(); i++) {
            std::cout << i << ":";
            for (const auto& item : bucket_list[i])
                std::cout << " (" << item.first << "," << item.second << ")";
            std::cout << "\n";
        }
    }
    /**
     * \brief Get iterator to the first element
     * \complexity Worst: O(m) (m = # of buckets) Best: O(1)
     */
    iterator begin() {
        if (empty())
            return end();
        return iterator(this,0,bucket_list[0].begin());
    }
    /**
     * \brief Get iterator to past-the-last element
     * \complexity O(1)
     */
    iterator end() {
        return iterator(this, -1,{});
    }

    /**
     * \brief Get iterator to the first element
     * \complexity Worst: O(m) (m = # of buckets) Best: O(1)
     */
    const_iterator begin() const {
        if (empty())
            return end();
        return const_iterator(this,0,bucket_list[0].begin());
    }
    /**
     * \brief Get iterator to past-the-last element
     * \complexity O(1)
     */
    const_iterator end() const {
        return const_iterator(this, -1,{});
    }

    /**
     * \brief Get iterator to the first element
     * \complexity Worst: O(m) (m = # of buckets) Best: O(1)
     */
    const_iterator cbegin() const {
        return begin();
    }
    /**
     * \brief Get iterator to past-the-last element
     * \complexity O(1)
     */
    const_iterator cend() const {
        return end();
    }

    /**
     * \brief Number of elements
     * \complexity O(1)
     */
    int size() const {
        return size_;
    }

    /**
     * \brief Whether the table is empty
     * \complexity O(1)
     */
    bool empty() const {
        return size_ == 0;
    }

    /**
     * \brief Number of buckets
     * \complexity O(1)
     */
    int bucket_count() const {
        return bucket_list.size();
    }

private:
    /**
     * \brief Get bucket index (hash value) from a key
     * \brief O(1)
     */
    hash_type KeyToBucket(const key_type& key) const{
        return std::hash<key_type>()(key) % bucket_list.size();
    }

    /**
     * \brief Find an iterator matching the key for the given bucket
     * \complexity Best: O(1) Worst: O(n) Always: O(m) where m is the # of items in the bucket
     */
    typename bucket_type::iterator find_key(bucket_type& bucket, const key_type& key) {
        return std::find_if(bucket.begin(),bucket.end(),
            [key](const item_type& item) { return item.first == key; } );
    }

    /**
     * \brief Find an iterator matching the key for the given bucket
     * \complexity Best: O(1) Worst: O(n) Always: O(m) where m is the # of items in the bucket
     */
    typename bucket_type::const_iterator find_key(const bucket_type& bucket, const key_type& key) const {
        return std::find_if(bucket.begin(),bucket.end(),
            [key](const item_type& item) { return item.first == key; } );
    }

    std::vector<bucket_type> bucket_list;
    int size_ = 0;
};
