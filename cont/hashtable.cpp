#include "hashtable.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;
// Вспомогательный хеш для строки


namespace
{
size_t rawHash(const string& keyValue) noexcept
{
    size_t hash = 146527;
    for (unsigned char characterValue : keyValue) { // перебор каждого символа
        hash ^= (hash << 5) + (hash >> 2) // 
              + static_cast<size_t>(characterValue);
    }
    return hash;
}
} 


// HashTable — цепная хеш-таблица


HashTable::HashTable()
    : bucketCountValue(8) // начальное количество бакетов
{
    bucketArray = new Node*[bucketCountValue];
    for (size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }
}

HashTable::HashTable(size_t initialBucketCount) // конструктор
    : bucketCountValue(initialBucketCount == 0 ? 1 : initialBucketCount) // начальное количество бакетов
{
    bucketArray = new Node*[bucketCountValue];
    for (size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr; // инициализация бакетов нулевыми указателями
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

void HashTable::freeBuckets() noexcept // освобождение памяти бакетов
{
    if (bucketArray == nullptr) {
        return;
    }
    for (size_t i = 0; i < bucketCountValue; ++i) {
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

size_t HashTable::hashString(const string& keyValue) const noexcept
{
    if (bucketCountValue == 0) {
        return 0;
    }
    return rawHash(keyValue) % bucketCountValue;
}

void HashTable::rehash(size_t newBucketCount)
{
    if (newBucketCount == 0) {
        newBucketCount = 1;
    }

    Node** oldBuckets = bucketArray;
    size_t oldCount = bucketCountValue;

    bucketArray = new Node*[newBucketCount];
    for (size_t i = 0; i < newBucketCount; ++i) {
        bucketArray[i] = nullptr;
    }
    bucketCountValue = newBucketCount;
    elementCount     = 0;

    // Перехэшируем элементы, сохраняя относительный порядок в списках
    for (size_t i = 0; i < oldCount; ++i) {
        Node* current = oldBuckets[i];
        while (current != nullptr) {
            Node* nextNode = current->getNext();

            const string& keyValue = current->getKey();
            const string& valueValue = current->getValue();

            size_t index = hashString(keyValue);

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
    for (size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }

    for (size_t i = 0; i < bucketCountValue; ++i) {
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
    for (size_t i = 0; i < bucketCountValue; ++i) {
        bucketArray[i] = nullptr;
    }

    for (size_t i = 0; i < bucketCountValue; ++i) {
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

void HashTable::insert(const string& keyValue, const string& valueValue)
{
    if (bucketCountValue == 0) {
        rehash(1);
    }

    const size_t index = hashString(keyValue);
    Node* current = bucketArray[index];

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            current->getValueRef() = valueValue;
            return;
        }
        current = current->getNext();
    }

    Node* newNode = new Node(keyValue, valueValue, bucketArray[index]);
    bucketArray[index] = newNode;
    ++elementCount;

    // если load factor > 0.75 — увеличиваем
    if (elementCount * 4 > bucketCountValue * 3) {
        rehash(bucketCountValue * 2);
    }
}

void HashTable::erase(const string& keyValue)
{
    if (bucketArray == nullptr || bucketCountValue == 0 || elementCount == 0) {
        return;
    }

    const size_t index = hashString(keyValue);
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

string* HashTable::find(const string& keyValue)
{
    if (bucketArray == nullptr || bucketCountValue == 0) {
        return nullptr;
    }

    const size_t index = hashString(keyValue);
    Node* current = bucketArray[index];

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            return &current->getValueRef();
        }
        current = current->getNext();
    }
    return nullptr;
}

const string* HashTable::find(const string& keyValue) const
{
    if (bucketArray == nullptr || bucketCountValue == 0) {
        return nullptr;
    }

    const size_t index = hashString(keyValue);
    Node* current = bucketArray[index];

    while (current != nullptr) {
        if (current->getKey() == keyValue) {
            return &current->getValueRef();
        }
        current = current->getNext();
    }
    return nullptr;
}

string& HashTable::operator[](const string& keyValue)
{
    string* valuePtr = find(keyValue);
    if (valuePtr != nullptr) {
        return *valuePtr;
    }

    insert(keyValue, string{});
    valuePtr = find(keyValue);
    // по логике теперь точно не null
    return *valuePtr;
}

size_t HashTable::size() const noexcept
{
    return elementCount;
}

bool HashTable::empty() const noexcept
{
    return elementCount == 0;
}

size_t HashTable::bucketCount() const noexcept
{
    return bucketCountValue;
}

void HashTable::print() const
{
    cout << "HashTable(size=" << elementCount
              << ", buckets=" << bucketCountValue << ")\n";
    for (size_t i = 0; i < bucketCountValue; ++i) {
        cout << "  [" << i << "]: ";
        Node* current = bucketArray[i];
        while (current != nullptr) {
            cout << "(" << current->getKey()
                      << " -> " << current->getValue() << ")";
            if (current->getNext() != nullptr) {
                cout << " -> ";
            }
            current = current->getNext();
        }
        cout << "\n";
    }
}

//  текстовая сериализация 

void HashTable::serializeText(ostream& outStream) const
{
    outStream << elementCount << '\n'; // записываем количество элементов
    for (size_t i = 0; i < bucketCountValue; ++i) { // перебираем бакеты
        Node* current = bucketArray[i]; // текущий узел в списке бакета
        while (current != nullptr) { 
            outStream << current->getKey() << '\t' << current->getValue() << '\n'; // записываем ключ и значение
            current = current->getNext();
        }
    }
}

string HashTable::serialize() const
{
    ostringstream oss; // используем строковый поток для накопления данных
    serializeText(oss); // сериализуем данные в поток
    return oss.str(); // возвращаем строковое представление
}

void HashTable::deserializeText(istream& inStream)
{
    clear(); // очищаем текущие данные

    size_t declaredCount = 0;
    if (!(inStream >> declaredCount)) {
        // не смогли прочитать count — считаем, что таблица остаётся пустой
        inStream.clear();
        return;
    }
    inStream.ignore(numeric_limits<streamsize>::max(), '\n'); // пропускаем остаток строки после числа

    for (size_t i = 0; i < declaredCount; ++i) {
        string keyValue;
        string valueValue;

        if (!getline(inStream, keyValue, '\t')) {
            throw runtime_error("HashTable::deserializeText: cannot read key");
        }
        if (!getline(inStream, valueValue)) {
            throw runtime_error("HashTable::deserializeText: cannot read value");
        }

        const size_t index = hashString(keyValue); // вычисляем индекс бакета

        Node* newNode = new Node(keyValue, valueValue, nullptr); // создаём новый узел
        if (bucketArray[index] == nullptr) { // если бакет пустой
            bucketArray[index] = newNode;// вставляем новый узел
        } else { // если бакет не пустой, добавляем в конец списка
            Node* tail = bucketArray[index]; // находим конец списка 
            while (tail->getNext() != nullptr) { // перебираем до конца
                tail = tail->getNext(); // переходим к следующему узлу
            }
            tail->getNextRef() = newNode; // добавляем новый узел в конец списка
        }
        ++elementCount;
    }
}

void HashTable::deserialize(const string& textData)
{
    istringstream iss(textData); // создаём поток из строки
    deserializeText(iss); // десериализуем данные из потока
}

//  бинарная сериализация 

void HashTable::serializeBinary(ostream& outStream) const   
{
    const uint64_t count64 = static_cast<uint64_t>(elementCount); // записываем количество элементов в 8 байт
    outStream.write(reinterpret_cast<const char*>(&count64), sizeof(count64)); // запись количества элементов

    for (size_t i = 0; i < bucketCountValue; ++i) { // перебираем бакеты
        Node* current = bucketArray[i];
        while (current != nullptr) {
            const string& keyValue = current->getKey();
            const string& valueValue = current->getValue();

            const uint64_t keySize = static_cast<uint64_t>(keyValue.size()); // размер ключа
            const uint64_t valSize = static_cast<uint64_t>(valueValue.size()); // размер значения

            outStream.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize)); // запись размера ключа
            if (keySize > 0) { 
                outStream.write(keyValue.data(), static_cast<streamsize>(keySize)); // запись данных ключа
            }

            outStream.write(reinterpret_cast<const char*>(&valSize), sizeof(valSize));
            if (valSize > 0) { 
                outStream.write(valueValue.data(), static_cast<streamsize>(valSize));
            }

            current = current->getNext(); // переходим к следующему узлу
        }
    }

    if (!outStream) {
        throw runtime_error("HashTable::serializeBinary: write error");
    }
}

void HashTable::deserializeBinary(istream& inStream)
{
    clear();

    uint64_t count64 = 0;
    inStream.read(reinterpret_cast<char*>(&count64), sizeof(count64)); // чтение количества элементов
    if (!inStream) {
        throw runtime_error("HashTable::deserializeBinary: cannot read count");
    }

    for (uint64_t i = 0; i < count64; ++i) { // перебор элементов
        uint64_t keySize = 0;
        inStream.read(reinterpret_cast<char*>(&keySize), sizeof(keySize)); // чтение размера ключа
        if (!inStream) {
            throw runtime_error("HashTable::deserializeBinary: cannot read key size");
        }

        string keyValue;
        keyValue.resize(static_cast<size_t>(keySize)); // выделение места под ключ
        if (keySize > 0) { // чтение данных ключа
            inStream.read(keyValue.data(),
                          static_cast<streamsize>(keySize));
            if (!inStream) {
                throw runtime_error("HashTable::deserializeBinary: cannot read key data");
            }
        }

        uint64_t valSize = 0;
        inStream.read(reinterpret_cast<char*>(&valSize), sizeof(valSize));
        if (!inStream) {
            throw runtime_error("HashTable::deserializeBinary: cannot read value size");
        }

        string valueValue;
        valueValue.resize(static_cast<size_t>(valSize));
        if (valSize > 0) {
            inStream.read(valueValue.data(),
                          static_cast<streamsize>(valSize));
            if (!inStream) {
                throw runtime_error("HashTable::deserializeBinary: cannot read value data");
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

HashTableOpen::HashTableOpen(size_t initialCapacity)
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

size_t HashTableOpen::hashString(const string& keyValue) const noexcept
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
    for (size_t i = 0; i < capacityValue; ++i) {
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
    for (size_t i = 0; i < capacityValue; ++i) {
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

size_t HashTableOpen::findSlotForInsert(const string& keyValue) const noexcept
{
    size_t index = hashString(keyValue);
    size_t start = index;

    while (tableArray[index].isOccupied && !tableArray[index].isDeleted
           && tableArray[index].key != keyValue) {
        index = (index + 1) % capacityValue;
        if (index == start) {
            break;
        }
    }
    return index;
}

size_t HashTableOpen::findSlotForKey(const string& keyValue) const noexcept
{
    size_t index = hashString(keyValue);
    size_t start = index;

    while (tableArray[index].isOccupied) {
        if (!tableArray[index].isDeleted && tableArray[index].key == keyValue) {
            return index;
        }
        index = (index + 1) % capacityValue;
        if (index == start) {
            break;
        }
    }
    return static_cast<size_t>(-1);
}

void HashTableOpen::rehash(size_t newCapacity)
{
    if (newCapacity == 0) {
        newCapacity = 1;
    }

    Cell* oldTable      = tableArray;
    size_t oldCap  = capacityValue;

    tableArray    = new Cell[newCapacity];
    capacityValue = newCapacity;
    elementCount  = 0;

    for (size_t i = 0; i < oldCap; ++i) {
        if (oldTable[i].isOccupied && !oldTable[i].isDeleted) {
            insert(oldTable[i].key, oldTable[i].value);
        }
    }

    delete[] oldTable;
}

void HashTableOpen::insert(const string& keyValue, const string& valueValue)
{
    if (capacityValue == 0) {
        rehash(1);
    }

    if (elementCount * 2 >= capacityValue) {
        rehash(capacityValue * 2);
    }

    const size_t index = findSlotForInsert(keyValue);
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

void HashTableOpen::erase(const string& keyValue)
{
    const size_t index = findSlotForKey(keyValue);
    if (index == static_cast<size_t>(-1)) {
        return;
    }
    Cell& cell    = tableArray[index];
    cell.isDeleted = true;
    --elementCount;
}

string* HashTableOpen::find(const string& keyValue)
{
    const size_t index = findSlotForKey(keyValue);
    if (index == static_cast<size_t>(-1)) {
        return nullptr;
    }
    return &tableArray[index].value;
}

const string* HashTableOpen::find(const string& keyValue) const
{
    const size_t index = findSlotForKey(keyValue);
    if (index == static_cast<size_t>(-1)) {
        return nullptr;
    }
    return &tableArray[index].value;
}

string& HashTableOpen::operator[](const string& keyValue)
{
    string* valuePtr = find(keyValue);
    if (valuePtr != nullptr) {
        return *valuePtr;
    }
    insert(keyValue, string{});
    valuePtr = find(keyValue);
    return *valuePtr;
}

size_t HashTableOpen::size() const noexcept
{
    return elementCount;
}

bool HashTableOpen::empty() const noexcept
{
    return elementCount == 0;
}

size_t HashTableOpen::capacity() const noexcept
{
    return capacityValue;
}

void HashTableOpen::clear() noexcept
{
    for (size_t i = 0; i < capacityValue; ++i) {
        tableArray[i].key.clear();
        tableArray[i].value.clear();
        tableArray[i].isOccupied = false;
        tableArray[i].isDeleted  = false;
    }
    elementCount = 0;
}

void HashTableOpen::print() const
{
    cout << "HashTableOpen(size=" << elementCount
              << ", capacity=" << capacityValue << ")\n";
    for (size_t i = 0; i < capacityValue; ++i) {
        const Cell& cell = tableArray[i];
        cout << "  [" << i << "]: ";
        if (!cell.isOccupied) {
            cout << "EMPTY";
        } else if (cell.isDeleted) {
            cout << "DELETED";
        } else {
            cout << "(" << cell.key << " -> " << cell.value << ")";
        }
        cout << "\n";
    }
}

//  текстовая сериализация 

void HashTableOpen::serializeText(ostream& outStream) const
{
    outStream << elementCount << '\n';
    for (size_t i = 0; i < capacityValue; ++i) {
        const Cell& cell = tableArray[i];
        if (cell.isOccupied && !cell.isDeleted) {
            outStream << cell.key << '\t' << cell.value << '\n';
        }
    }
}

string HashTableOpen::serialize() const
{
    ostringstream oss;
    serializeText(oss);
    return oss.str();
}

void HashTableOpen::deserializeText(istream& inStream)
{
    clear();

    size_t declaredCount = 0;
    if (!(inStream >> declaredCount)) {
        inStream.clear();
        return;
    }
    inStream.ignore(numeric_limits<streamsize>::max(), '\n');

    for (size_t i = 0; i < declaredCount; ++i) {
        string keyValue;
        string valueValue;

        if (!getline(inStream, keyValue, '\t')) {
            throw runtime_error("HashTableOpen::deserializeText: cannot read key");
        }
        if (!getline(inStream, valueValue)) {
            throw runtime_error("HashTableOpen::deserializeText: cannot read value");
        }
        insert(keyValue, valueValue);
    }
}

void HashTableOpen::deserialize(const string& textData)
{
    istringstream iss(textData);
    deserializeText(iss);
}

//  бинарная сериализация 

void HashTableOpen::serializeBinary(ostream& outStream) const
{
    const uint64_t count64 = static_cast<uint64_t>(elementCount);
    outStream.write(reinterpret_cast<const char*>(&count64), sizeof(count64));

    for (size_t i = 0; i < capacityValue; ++i) {
        const Cell& cell = tableArray[i];
        if (cell.isOccupied && !cell.isDeleted) {
            const uint64_t keySize =
                static_cast<uint64_t>(cell.key.size());
            const uint64_t valSize =
                static_cast<uint64_t>(cell.value.size());

            outStream.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            if (keySize > 0) {
                outStream.write(cell.key.data(),
                                static_cast<streamsize>(keySize));
            }

            outStream.write(reinterpret_cast<const char*>(&valSize), sizeof(valSize));
            if (valSize > 0) {
                outStream.write(cell.value.data(),
                                static_cast<streamsize>(valSize));
            }
        }
    }

    if (!outStream) {
        throw runtime_error("HashTableOpen::serializeBinary: write error");
    }
}

void HashTableOpen::deserializeBinary(istream& inStream)
{
    clear();

    uint64_t count64 = 0;
    inStream.read(reinterpret_cast<char*>(&count64), sizeof(count64));
    if (!inStream) {
        throw runtime_error("HashTableOpen::deserializeBinary: cannot read count");
    }

    for (uint64_t i = 0; i < count64; ++i) {
        uint64_t keySize = 0;
        inStream.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        if (!inStream) {
            throw runtime_error("HashTableOpen::deserializeBinary: cannot read key size");
        }

        string keyValue;
        keyValue.resize(static_cast<size_t>(keySize));
        if (keySize > 0) {
            inStream.read(keyValue.data(),
                          static_cast<streamsize>(keySize));
            if (!inStream) {
                throw runtime_error("HashTableOpen::deserializeBinary: cannot read key data");
            }
        }

        uint64_t valSize = 0;
        inStream.read(reinterpret_cast<char*>(&valSize), sizeof(valSize));
        if (!inStream) {
            throw runtime_error("HashTableOpen::deserializeBinary: cannot read value size");
        }

        string valueValue;
        valueValue.resize(static_cast<size_t>(valSize));
        if (valSize > 0) {
            inStream.read(valueValue.data(),
                          static_cast<streamsize>(valSize));
            if (!inStream) {
                throw runtime_error("HashTableOpen::deserializeBinary: cannot read value data");
            }
        }

        insert(keyValue, valueValue);
    }
}