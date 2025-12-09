#include "array.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <cstdint>

// конструкторы / деструктор 

MyArray::MyArray(std::size_t initialCapacity)
    : dataPtr(nullptr),
      capacity(initialCapacity),
      length(0)
{
    if (capacity == 0) {
        capacity = 1;
    }

    dataPtr = new std::string[capacity];
}

MyArray::~MyArray()
{
    delete[] dataPtr;
    dataPtr = nullptr;
    capacity = 0;
    length = 0;
}

// копирующий конструктор 

MyArray::MyArray(const MyArray& other)
    : dataPtr(nullptr),
      capacity(other.capacity),
      length(other.length)
{
    dataPtr = new std::string[capacity];
    for (std::size_t i = 0; i < length; ++i) {
        dataPtr[i] = other.dataPtr[i];
    }
}

// move-конструктор 

MyArray::MyArray(MyArray&& other) noexcept
    : dataPtr(other.dataPtr),
      capacity(other.capacity),
      length(other.length)
{
    other.dataPtr = nullptr;
    other.capacity = 0;
    other.length = 0;
}

// копирующее присваивание 

MyArray& MyArray::operator=(const MyArray& other)
{
    if (this == &other) {
        return *this;
    }

    MyArray tmp(other);
    swap(tmp);
    return *this;
}

// move-присваивание 

MyArray& MyArray::operator=(MyArray&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    delete[] dataPtr;

    dataPtr = other.dataPtr;
    capacity = other.capacity;
    length = other.length;

    other.dataPtr = nullptr;
    other.capacity = 0;
    other.length = 0;

    return *this;
}

// swap 

void MyArray::swap(MyArray& other) noexcept
{
    using std::swap;
    swap(dataPtr, other.dataPtr);
    swap(capacity, other.capacity);
    swap(length, other.length);
}

// базовые операции 

void MyArray::pushBack(const std::string& value)
{
    if (length == capacity) {
        resize(capacity * 2);
    }
    dataPtr[length++] = value;
}

void MyArray::insert(std::size_t index, const std::string& value)
{
    if (index > length) {
        throw std::out_of_range("MyArray::insert: error");
    }

    if (length == capacity) {
        resize(capacity * 2);
    }

    for (std::size_t i = length; i > index; --i) {
        dataPtr[i] = dataPtr[i - 1];
    }

    dataPtr[index] = value;
    ++length;
}

void MyArray::removeAt(std::size_t index)
{
    if (index >= length) {
        throw std::out_of_range("MyArray::removeAt: error");
    }

    for (std::size_t i = index; i + 1 < length; ++i) {
        dataPtr[i] = dataPtr[i + 1];
    }
    --length;
}

std::string& MyArray::at(std::size_t index)
{
    if (index >= length) {
        throw std::out_of_range("MyArray::at: error");
    }
    return dataPtr[index];
}

const std::string& MyArray::at(std::size_t index) const
{
    if (index >= length) {
        throw std::out_of_range("MyArray::at: error");
    }
    return dataPtr[index];
}

void MyArray::set(std::size_t index, const std::string& value)
{
    if (index >= length) {
        throw std::out_of_range("MyArray::set: error");
    }
    dataPtr[index] = value;
}

// operator[] 

std::string& MyArray::operator[](std::size_t index) noexcept
{
    return dataPtr[index];
}

const std::string& MyArray::operator[](std::size_t index) const noexcept
{
    return dataPtr[index];
}

// вспомогательные методы 

std::size_t MyArray::size() const noexcept
{
    return length;
}

std::size_t MyArray::getCapacity() const noexcept
{
    return capacity;
}

bool MyArray::empty() const noexcept
{
    return length == 0;
}

void MyArray::print() const
{
    std::cout << "[";
    for (std::size_t i = 0; i < length; ++i) {
        std::cout << dataPtr[i];
        if (i + 1 < length) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
}

void MyArray::resize(std::size_t newCapacity)
{
    if (newCapacity == 0) {
        newCapacity = 1;
    }

    length = std::min(newCapacity, length);

    std::string* newData = new std::string[newCapacity];
    for (std::size_t i = 0; i < length; ++i) {
        newData[i] = dataPtr[i];
    }

    delete[] dataPtr;
    dataPtr = newData;
    capacity = newCapacity;
}

// текстовая сериализация через поток 

void MyArray::serializeText(std::ostream& outputStream) const
{
    outputStream << length << '\n';
    for (std::size_t i = 0; i < length; ++i) {
        outputStream << dataPtr[i] << '\n';
    }
}

void MyArray::deserializeText(std::istream& inputStream)
{
    std::size_t newLength = 0;
    if (!(inputStream >> newLength)) {
        throw std::runtime_error("MyArray::deserializeText: error");
    }

    inputStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (newLength > capacity) {
        resize(newLength);
    }

    length = newLength; 
    for (std::size_t i = 0; i < length; ++i) {
        if (!std::getline(inputStream, dataPtr[i])) { // чтение строки с учётом пробелов
            throw std::runtime_error("MyArray::deserializeText: error");
        }
    }
}

// текстовая сериализация в строку

std::string MyArray::serialize() const
{
    std::ostringstream oss;
    serializeText(oss);
    return oss.str();
}

void MyArray::deserialize(const std::string& dataString)
{
    std::istringstream iss(dataString);
    deserializeText(iss);
}

// бинарная сериализация 

void MyArray::serializeBinary(std::ostream& outputStream) const
{
    std::uint64_t len = static_cast<std::uint64_t>(length); // запись количества элементов
    outputStream.write(reinterpret_cast<const char*>(&len), sizeof(len)); // запись длины

    for (std::size_t i = 0; i < length; ++i) { // запись каждого элемента
        const std::string& text = dataPtr[i]; // текущая строка
        std::uint64_t sizeValue = static_cast<std::uint64_t>(text.size()); // размер строки
        outputStream.write(reinterpret_cast<const char*>(&sizeValue), sizeof(sizeValue)); // запись размера строки
        if (sizeValue > 0) { 
            outputStream.write(text.data(), static_cast<std::streamsize>(sizeValue)); // запись данных строки
        }
    }

    if (!outputStream) {
        throw std::runtime_error("MyArray::serializeBinary: error");
    }
}

void MyArray::deserializeBinary(std::istream& inputStream)
{
    std::uint64_t len = 0;
    inputStream.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (!inputStream) {
        throw std::runtime_error("MyArray::deserializeBinary: error");
    }

    if (len > static_cast<std::uint64_t>(capacity)) {
        resize(static_cast<std::size_t>(len));
    }

    length = static_cast<std::size_t>(len);

    for (std::size_t i = 0; i < length; ++i) {
        std::uint64_t sizeValue = 0;
        inputStream.read(reinterpret_cast<char*>(&sizeValue), sizeof(sizeValue));
        if (!inputStream) {
            throw std::runtime_error("MyArray::deserializeBinary: error");
        }

        std::string tmp;
        tmp.resize(static_cast<std::size_t>(sizeValue));
        if (sizeValue > 0) {
            inputStream.read(tmp.data(), static_cast<std::streamsize>(sizeValue));
            if (!inputStream) {
                throw std::runtime_error("MyArray::deserializeBinary: error");
            }
        }
        dataPtr[i] = std::move(tmp);
    }
}
