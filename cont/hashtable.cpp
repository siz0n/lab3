#include "hashtable.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>


// Вспомогательный хеш для строки


namespace
{
std::size_t rawHash(const std::string& keyValue) noexcept
{
    std::size_t hash = 146527;
    for (unsigned char characterValue : keyValue) {
        hash ^= (hash << 5) + (hash >> 2)
              + static_cast<std::size_t>(characterValue);
    }
    return hash;
}
} 


// HashTable — цепная хеш-таблица


HashTable::HashTable()
    : bucketCountValue(8)
{
    bucketArray = new Node*[bucketCountValue];
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }
}

HashTable::HashTable(std::size_t initialBucketCount)
    : bucketCountValue(initialBucketCount == 0 ? 1 : initialBucketCount)
{
    bucketArray = new Node*[bucketCountValue];
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }
}

HashTable::~HashTable()
{
    clear();
    delete[] bucketArray;
    bucketArray = nullptr;
    bucketCountValue = 0;
    elementCount = 0;
}

void HashTable::freeBuckets() noexcept
{
    if (bucketArray == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        Node* current = bucketArray[i];
        while (current != nullptr) {
            Node* toDelete = current;
            current = current->getNext();
            delete toDelete;
        }
        bucketArray[i] = nullptr;
    }
    elementCount = 0;
}

void HashTable::clear() noexcept
{
    freeBuckets();
}

std::size_t HashTable::hashString(const std::string& keyValue) const noexcept
{
    if (bucketCountValue == 0) {
        return 0;
    }
    return rawHash(keyValue) % bucketCountValue;
}

void HashTable::rehash(std::size_t newBucketCount)
{
    if (newBucketCount == 0) {
        newBucketCount = 1;
    }

    Node** oldBuckets = bucketArray;
    std::size_t oldCount = bucketCountValue;

    bucketArray = new Node*[newBucketCount];
    for (std::size_t i = 0; i < newBucketCount; ++i) {
        bucketArray[i] = nullptr;
    }
    bucketCountValue = newBucketCount;
    elementCount     = 0;

    // Перехэшируем элементы, сохраняя относительный порядок в списках
    for (std::size_t i = 0; i < oldCount; ++i) {
        Node* current = oldBuckets[i];
        while (current != nullptr) {
            Node* nextNode = current->getNext();

            const std::string& keyValue = current->getKey();
            const std::string& valueValue = current->getValue();

            std::size_t index = hashString(keyValue);

            // вставляем в начало списка нового бакета
            current->getNextRef() = bucketArray[index];
            current->getValueRef() = valueValue;
            bucketArray[index] = current;

            ++elementCount;
            current = nextNode;
        }
    }

    delete[] oldBuckets;
}

//  Rule of Five 

HashTable::HashTable(const HashTable& other)
    : bucketCountValue(other.bucketCountValue)
{
    bucketArray = new Node*[bucketCountValue];
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }

    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        Node* current = other.bucketArray[i];
        // копируем по порядку, чтобы сохранить структуру
        while (current != nullptr) {
            insert(current->getKey(), current->getValue());
            current = current->getNext();
        }
    }
}

HashTable::HashTable(HashTable&& other) noexcept
    : bucketArray(other.bucketArray),
      bucketCountValue(other.bucketCountValue),
      elementCount(other.elementCount)
{
    other.bucketArray = nullptr;
    other.bucketCountValue = 0;
    other.elementCount = 0;
}

HashTable& HashTable::operator=(const HashTable& other)
{
    if (this == &other) {
        return *this;
    }

    clear();
    delete[] bucketArray;

    bucketCountValue = other.bucketCountValue;
    elementCount = 0;

    bucketArray = new Node*[bucketCountValue];
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }

    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        Node* current = other.bucketArray[i];
        while (current != nullptr) {
            insert(current->getKey(), current->getValue());
            current = current->getNext();
        }
    }

    return *this;
}

HashTable& HashTable::operator=(HashTable&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    clear();
    delete[] bucketArray;

    bucketArray = other.bucketArray;
    bucketCountValue = other.bucketCountValue;
    elementCount = other.elementCount;

    other.bucketArray = nullptr;
    other.bucketCountValue = 0;
    other.elementCount = 0;

    return *this;
}

//  основные операции 

void HashTable::insert(const std::string& keyValue, const std::string& valueValue)
{
    if (bucketCountValue == 0) {
        rehash(1);
    }

    const std::size_t index = hashString(keyValue);
    Node* current = bucketArray[index];

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            current->getValueRef() = valueValue;
            return;
        }
        current = current->getNext();
    }

    Node* newNode      = new Node(keyValue, valueValue, bucketArray[index]);
    bucketArray[index] = newNode;
    ++elementCount;

    // если load factor > 0.75 — увеличиваем
    if (elementCount * 4 > bucketCountValue * 3) {
        rehash(bucketCountValue * 2);
    }
}

void HashTable::erase(const std::string& keyValue)
{
    if (bucketArray == nullptr || bucketCountValue == 0 || elementCount == 0) {
        return;
    }

    const std::size_t index = hashString(keyValue);
    Node* current = bucketArray[index];
    Node* previous = nullptr;

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            if (previous == nullptr) {
                bucketArray[index] = current->getNext();
            } else {
                previous->getNextRef() = current->getNext();
            }
            delete current;
            --elementCount;
            return;
        }
        previous = current;
        current = current->getNext();
    }
}

std::string* HashTable::find(const std::string& keyValue)
{
    if (bucketArray == nullptr || bucketCountValue == 0) {
        return nullptr;
    }

    const std::size_t index = hashString(keyValue);
    Node* current = bucketArray[index];

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            return &current->getValueRef();
        }
        current = current->getNext();
    }
    return nullptr;
}

const std::string* HashTable::find(const std::string& keyValue) const
{
    if (bucketArray == nullptr || bucketCountValue == 0) {
        return nullptr;
    }

    const std::size_t index = hashString(keyValue);
    Node* current = bucketArray[index];

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            return &current->getValueRef();
        }
        current = current->getNext();
    }
    return nullptr;
}

std::string& HashTable::operator[](const std::string& keyValue)
{
    std::string* valuePtr = find(keyValue);
    if (valuePtr != nullptr) {
        return *valuePtr;
    }

    insert(keyValue, std::string{});
    valuePtr = find(keyValue);
    // по логике теперь точно не null
    return *valuePtr;
}

std::size_t HashTable::size() const noexcept
{
    return elementCount;
}

bool HashTable::empty() const noexcept
{
    return elementCount == 0;
}

std::size_t HashTable::bucketCount() const noexcept
{
    return bucketCountValue;
}

void HashTable::print() const
{
    std::cout << "HashTable(size=" << elementCount
              << ", buckets=" << bucketCountValue << ")\n";
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        std::cout << "  [" << i << "]: ";
        Node* current = bucketArray[i];
        while (current != nullptr) {
            std::cout << "(" << current->getKey()
                      << " -> " << current->getValue() << ")";
            if (current->getNext() != nullptr) {
                std::cout << " -> ";
            }
            current = current->getNext();
        }
        std::cout << "\n";
    }
}

//  текстовая сериализация 

void HashTable::serializeText(std::ostream& outStream) const
{
    outStream << elementCount << '\n';
    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        Node* current = bucketArray[i];
        while (current != nullptr) {
            outStream << current->getKey() << '\t' << current->getValue() << '\n';
            current = current->getNext();
        }
    }
}

std::string HashTable::serialize() const
{
    std::ostringstream oss;
    serializeText(oss);
    return oss.str();
}

void HashTable::deserializeText(std::istream& inStream)
{
    clear();

    std::size_t declaredCount = 0;
    if (!(inStream >> declaredCount)) {
        // не смогли прочитать count — считаем, что таблица остаётся пустой
        inStream.clear();
        return;
    }
    inStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (std::size_t i = 0; i < declaredCount; ++i) {
        std::string keyValue;
        std::string valueValue;

        if (!std::getline(inStream, keyValue, '\t')) {
            throw std::runtime_error("HashTable::deserializeText: cannot read key");
        }
        if (!std::getline(inStream, valueValue)) {
            throw std::runtime_error("HashTable::deserializeText: cannot read value");
        }

        const std::size_t index = hashString(keyValue);

        Node* newNode = new Node(keyValue, valueValue, nullptr);
        if (bucketArray[index] == nullptr) {
            bucketArray[index] = newNode;
        } else {
            Node* tail = bucketArray[index];
            while (tail->getNext() != nullptr) {
                tail = tail->getNext();
            }
            tail->getNextRef() = newNode;
        }
        ++elementCount;
    }
}

void HashTable::deserialize(const std::string& textData)
{
    std::istringstream iss(textData);
    deserializeText(iss);
}

//  бинарная сериализация 

void HashTable::serializeBinary(std::ostream& outStream) const
{
    const std::uint64_t count64 = static_cast<std::uint64_t>(elementCount);
    outStream.write(reinterpret_cast<const char*>(&count64), sizeof(count64));

    for (std::size_t i = 0; i < bucketCountValue; ++i) {
        Node* current = bucketArray[i];
        while (current != nullptr) {
            const std::string& keyValue   = current->getKey();
            const std::string& valueValue = current->getValue();

            const std::uint64_t keySize =
                static_cast<std::uint64_t>(keyValue.size());
            const std::uint64_t valSize =
                static_cast<std::uint64_t>(valueValue.size());

            outStream.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            if (keySize > 0) {
                outStream.write(keyValue.data(),
                                static_cast<std::streamsize>(keySize));
            }

            outStream.write(reinterpret_cast<const char*>(&valSize), sizeof(valSize));
            if (valSize > 0) {
                outStream.write(valueValue.data(),
                                static_cast<std::streamsize>(valSize));
            }

            current = current->getNext();
        }
    }

    if (!outStream) {
        throw std::runtime_error("HashTable::serializeBinary: write error");
    }
}

void HashTable::deserializeBinary(std::istream& inStream)
{
    clear();

    std::uint64_t count64 = 0;
    inStream.read(reinterpret_cast<char*>(&count64), sizeof(count64));
    if (!inStream) {
        throw std::runtime_error("HashTable::deserializeBinary: cannot read count");
    }

    for (std::uint64_t i = 0; i < count64; ++i) {
        std::uint64_t keySize = 0;
        inStream.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        if (!inStream) {
            throw std::runtime_error("HashTable::deserializeBinary: cannot read key size");
        }

        std::string keyValue;
        keyValue.resize(static_cast<std::size_t>(keySize));
        if (keySize > 0) {
            inStream.read(keyValue.data(),
                          static_cast<std::streamsize>(keySize));
            if (!inStream) {
                throw std::runtime_error("HashTable::deserializeBinary: cannot read key data");
            }
        }

        std::uint64_t valSize = 0;
        inStream.read(reinterpret_cast<char*>(&valSize), sizeof(valSize));
        if (!inStream) {
            throw std::runtime_error("HashTable::deserializeBinary: cannot read value size");
        }

        std::string valueValue;
        valueValue.resize(static_cast<std::size_t>(valSize));
        if (valSize > 0) {
            inStream.read(valueValue.data(),
                          static_cast<std::streamsize>(valSize));
            if (!inStream) {
                throw std::runtime_error("HashTable::deserializeBinary: cannot read value data");
            }
        }

        insert(keyValue, valueValue);
    }
}


// HashTableOpen — открытая адресация


HashTableOpen::HashTableOpen()
    : capacityValue(8)
{
    tableArray = new Cell[capacityValue];
}

HashTableOpen::HashTableOpen(std::size_t initialCapacity)
    : capacityValue(initialCapacity == 0 ? 1 : initialCapacity)
{
    tableArray = new Cell[capacityValue];
}

HashTableOpen::~HashTableOpen()
{
    delete[] tableArray;
    tableArray    = nullptr;
    capacityValue = 0;
    elementCount  = 0;
}

std::size_t HashTableOpen::hashString(const std::string& keyValue) const noexcept
{
    if (capacityValue == 0) {
        return 0;
    }
    return rawHash(keyValue) % capacityValue;
}

HashTableOpen::HashTableOpen(const HashTableOpen& other)
    : capacityValue(other.capacityValue),
      elementCount(other.elementCount)
{
    tableArray = new Cell[capacityValue];
    for (std::size_t i = 0; i < capacityValue; ++i) {
        tableArray[i] = other.tableArray[i];
    }
}

HashTableOpen::HashTableOpen(HashTableOpen&& other) noexcept
    : tableArray(other.tableArray),
      capacityValue(other.capacityValue),
      elementCount(other.elementCount)
{
    other.tableArray    = nullptr;
    other.capacityValue = 0;
    other.elementCount  = 0;
}

HashTableOpen& HashTableOpen::operator=(const HashTableOpen& other)
{
    if (this == &other) {
        return *this;
    }

    delete[] tableArray;

    capacityValue = other.capacityValue;
    elementCount  = other.elementCount;

    tableArray = new Cell[capacityValue];
    for (std::size_t i = 0; i < capacityValue; ++i) {
        tableArray[i] = other.tableArray[i];
    }

    return *this;
}

HashTableOpen& HashTableOpen::operator=(HashTableOpen&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    delete[] tableArray;

    tableArray    = other.tableArray;
    capacityValue = other.capacityValue;
    elementCount  = other.elementCount;

    other.tableArray    = nullptr;
    other.capacityValue = 0;
    other.elementCount  = 0;

    return *this;
}

std::size_t HashTableOpen::findSlotForInsert(const std::string& keyValue) const noexcept
{
    std::size_t index = hashString(keyValue);
    std::size_t start = index;

    while (tableArray[index].isOccupied && !tableArray[index].isDeleted
           && tableArray[index].key != keyValue) {
        index = (index + 1) % capacityValue;
        if (index == start) {
            break;
        }
    }
    return index;
}

std::size_t HashTableOpen::findSlotForKey(const std::string& keyValue) const noexcept
{
    std::size_t index = hashString(keyValue);
    std::size_t start = index;

    while (tableArray[index].isOccupied) {
        if (!tableArray[index].isDeleted && tableArray[index].key == keyValue) {
            return index;
        }
        index = (index + 1) % capacityValue;
        if (index == start) {
            break;
        }
    }
    return static_cast<std::size_t>(-1);
}

void HashTableOpen::rehash(std::size_t newCapacity)
{
    if (newCapacity == 0) {
        newCapacity = 1;
    }

    Cell* oldTable      = tableArray;
    std::size_t oldCap  = capacityValue;

    tableArray    = new Cell[newCapacity];
    capacityValue = newCapacity;
    elementCount  = 0;

    for (std::size_t i = 0; i < oldCap; ++i) {
        if (oldTable[i].isOccupied && !oldTable[i].isDeleted) {
            insert(oldTable[i].key, oldTable[i].value);
        }
    }

    delete[] oldTable;
}

void HashTableOpen::insert(const std::string& keyValue, const std::string& valueValue)
{
    if (capacityValue == 0) {
        rehash(1);
    }

    if (elementCount * 2 >= capacityValue) {
        rehash(capacityValue * 2);
    }

    const std::size_t index = findSlotForInsert(keyValue);
    Cell& cell              = tableArray[index];

    if (cell.isOccupied && !cell.isDeleted) {
        cell.value = valueValue;
        return;
    }

    cell.key        = keyValue;
    cell.value      = valueValue;
    cell.isOccupied = true;
    cell.isDeleted  = false;
    ++elementCount;
}

void HashTableOpen::erase(const std::string& keyValue)
{
    const std::size_t index = findSlotForKey(keyValue);
    if (index == static_cast<std::size_t>(-1)) {
        return;
    }
    Cell& cell    = tableArray[index];
    cell.isDeleted = true;
    --elementCount;
}

std::string* HashTableOpen::find(const std::string& keyValue)
{
    const std::size_t index = findSlotForKey(keyValue);
    if (index == static_cast<std::size_t>(-1)) {
        return nullptr;
    }
    return &tableArray[index].value;
}

const std::string* HashTableOpen::find(const std::string& keyValue) const
{
    const std::size_t index = findSlotForKey(keyValue);
    if (index == static_cast<std::size_t>(-1)) {
        return nullptr;
    }
    return &tableArray[index].value;
}

std::string& HashTableOpen::operator[](const std::string& keyValue)
{
    std::string* valuePtr = find(keyValue);
    if (valuePtr != nullptr) {
        return *valuePtr;
    }
    insert(keyValue, std::string{});
    valuePtr = find(keyValue);
    return *valuePtr;
}

std::size_t HashTableOpen::size() const noexcept
{
    return elementCount;
}

bool HashTableOpen::empty() const noexcept
{
    return elementCount == 0;
}

std::size_t HashTableOpen::capacity() const noexcept
{
    return capacityValue;
}

void HashTableOpen::clear() noexcept
{
    for (std::size_t i = 0; i < capacityValue; ++i) {
        tableArray[i].key.clear();
        tableArray[i].value.clear();
        tableArray[i].isOccupied = false;
        tableArray[i].isDeleted  = false;
    }
    elementCount = 0;
}

void HashTableOpen::print() const
{
    std::cout << "HashTableOpen(size=" << elementCount
              << ", capacity=" << capacityValue << ")\n";
    for (std::size_t i = 0; i < capacityValue; ++i) {
        const Cell& cell = tableArray[i];
        std::cout << "  [" << i << "]: ";
        if (!cell.isOccupied) {
            std::cout << "EMPTY";
        } else if (cell.isDeleted) {
            std::cout << "DELETED";
        } else {
            std::cout << "(" << cell.key << " -> " << cell.value << ")";
        }
        std::cout << "\n";
    }
}

//  текстовая сериализация 

void HashTableOpen::serializeText(std::ostream& outStream) const
{
    outStream << elementCount << '\n';
    for (std::size_t i = 0; i < capacityValue; ++i) {
        const Cell& cell = tableArray[i];
        if (cell.isOccupied && !cell.isDeleted) {
            outStream << cell.key << '\t' << cell.value << '\n';
        }
    }
}

std::string HashTableOpen::serialize() const
{
    std::ostringstream oss;
    serializeText(oss);
    return oss.str();
}

void HashTableOpen::deserializeText(std::istream& inStream)
{
    clear();

    std::size_t declaredCount = 0;
    if (!(inStream >> declaredCount)) {
        inStream.clear();
        return;
    }
    inStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (std::size_t i = 0; i < declaredCount; ++i) {
        std::string keyValue;
        std::string valueValue;

        if (!std::getline(inStream, keyValue, '\t')) {
            throw std::runtime_error("HashTableOpen::deserializeText: cannot read key");
        }
        if (!std::getline(inStream, valueValue)) {
            throw std::runtime_error("HashTableOpen::deserializeText: cannot read value");
        }
        insert(keyValue, valueValue);
    }
}

void HashTableOpen::deserialize(const std::string& textData)
{
    std::istringstream iss(textData);
    deserializeText(iss);
}

//  бинарная сериализация 

void HashTableOpen::serializeBinary(std::ostream& outStream) const
{
    const std::uint64_t count64 = static_cast<std::uint64_t>(elementCount);
    outStream.write(reinterpret_cast<const char*>(&count64), sizeof(count64));

    for (std::size_t i = 0; i < capacityValue; ++i) {
        const Cell& cell = tableArray[i];
        if (cell.isOccupied && !cell.isDeleted) {
            const std::uint64_t keySize =
                static_cast<std::uint64_t>(cell.key.size());
            const std::uint64_t valSize =
                static_cast<std::uint64_t>(cell.value.size());

            outStream.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            if (keySize > 0) {
                outStream.write(cell.key.data(),
                                static_cast<std::streamsize>(keySize));
            }

            outStream.write(reinterpret_cast<const char*>(&valSize), sizeof(valSize));
            if (valSize > 0) {
                outStream.write(cell.value.data(),
                                static_cast<std::streamsize>(valSize));
            }
        }
    }

    if (!outStream) {
        throw std::runtime_error("HashTableOpen::serializeBinary: write error");
    }
}

void HashTableOpen::deserializeBinary(std::istream& inStream)
{
    clear();

    std::uint64_t count64 = 0;
    inStream.read(reinterpret_cast<char*>(&count64), sizeof(count64));
    if (!inStream) {
        throw std::runtime_error("HashTableOpen::deserializeBinary: cannot read count");
    }

    for (std::uint64_t i = 0; i < count64; ++i) {
        std::uint64_t keySize = 0;
        inStream.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        if (!inStream) {
            throw std::runtime_error("HashTableOpen::deserializeBinary: cannot read key size");
        }

        std::string keyValue;
        keyValue.resize(static_cast<std::size_t>(keySize));
        if (keySize > 0) {
            inStream.read(keyValue.data(),
                          static_cast<std::streamsize>(keySize));
            if (!inStream) {
                throw std::runtime_error("HashTableOpen::deserializeBinary: cannot read key data");
            }
        }

        std::uint64_t valSize = 0;
        inStream.read(reinterpret_cast<char*>(&valSize), sizeof(valSize));
        if (!inStream) {
            throw std::runtime_error("HashTableOpen::deserializeBinary: cannot read value size");
        }

        std::string valueValue;
        valueValue.resize(static_cast<std::size_t>(valSize));
        if (valSize > 0) {
            inStream.read(valueValue.data(),
                          static_cast<std::streamsize>(valSize));
            if (!inStream) {
                throw std::runtime_error("HashTableOpen::deserializeBinary: cannot read value data");
            }
        }

        insert(keyValue, valueValue);
    }
}
