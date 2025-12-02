#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <utility>


//  HashTable — цепная хеш-таблица


class HashTable
{
private:
    class Node
    {
    public:
        Node(std::string keyValue,
             std::string valueValue,
             Node* nextNode) noexcept
            : key(std::move(keyValue)),
              value(std::move(valueValue)),
              next(nextNode)
        {
        }

        [[nodiscard]] const std::string& getKey() const noexcept
        {
            return key;
        }

        [[nodiscard]] const std::string& getValue() const noexcept
        {
            return value;
        }

        std::string& getValueRef() noexcept
        {
            return value;
        }

        [[nodiscard]] Node* getNext() const noexcept
        {
            return next;
        }

        Node*& getNextRef() noexcept
        {
            return next;
        }

    private:
        std::string key;
        std::string value;
        Node* next;
    };

    Node** bucketArray{nullptr};
    std::size_t bucketCountValue{0U};
    std::size_t elementCount{0U};

    void freeBuckets() noexcept;
    void rehash(std::size_t newBucketCount);
    [[nodiscard]] std::size_t hashString(const std::string& keyValue) const noexcept;

public:
    HashTable();
    explicit HashTable(std::size_t initialBucketCount);
    HashTable(const HashTable& other);
    HashTable(HashTable&& other) noexcept;
    HashTable& operator=(const HashTable& other);
    HashTable& operator=(HashTable&& other) noexcept;
    ~HashTable();

    void insert(const std::string& keyValue, const std::string& valueValue);
    void erase(const std::string& keyValue);

    [[nodiscard]] std::string* find(const std::string& keyValue);
    [[nodiscard]] const std::string* find(const std::string& keyValue) const;

    std::string& operator[](const std::string& keyValue);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t bucketCount() const noexcept;

    void clear() noexcept;
    void print() const;

    // екстовая сериализация 
    void serializeText(std::ostream& outStream) const;
    [[nodiscard]] std::string serialize() const;
    void deserializeText(std::istream& inStream);
    void deserialize(const std::string& textData);

    //  бинарная сериализация 
    void serializeBinary(std::ostream& outStream) const;
    void deserializeBinary(std::istream& inStream);
};



//ashTableOpen — открытая адресация


class HashTableOpen
{
private:
    struct Cell
    {
        std::string key;
        std::string value;
        bool isOccupied{false};
        bool isDeleted{false};
    };

    Cell* tableArray{nullptr};
    std::size_t capacityValue{0U};
    std::size_t elementCount{0U};

    [[nodiscard]] std::size_t hashString(const std::string& keyValue) const noexcept;
    [[nodiscard]] std::size_t findSlotForInsert(const std::string& keyValue) const noexcept;
    [[nodiscard]] std::size_t findSlotForKey(const std::string& keyValue) const noexcept;
    void rehash(std::size_t newCapacity);

public:
    HashTableOpen();
    explicit HashTableOpen(std::size_t initialCapacity);
    HashTableOpen(const HashTableOpen& other);
    HashTableOpen(HashTableOpen&& other) noexcept;
    HashTableOpen& operator=(const HashTableOpen& other);
    HashTableOpen& operator=(HashTableOpen&& other) noexcept;
    ~HashTableOpen();

    void insert(const std::string& keyValue, const std::string& valueValue);
    void erase(const std::string& keyValue);

    [[nodiscard]] std::string* find(const std::string& keyValue);
    [[nodiscard]] const std::string* find(const std::string& keyValue) const;

    std::string& operator[](const std::string& keyValue);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;

    void clear() noexcept;
    void print() const;

    //  текстовая сериализация
    void serializeText(std::ostream& outStream) const;
    [[nodiscard]] std::string serialize() const;
    void deserializeText(std::istream& inStream);
    void deserialize(const std::string& textData);

    //  бинарная сериализация 
    void serializeBinary(std::ostream& outStream) const;
    void deserializeBinary(std::istream& inStream);
};
