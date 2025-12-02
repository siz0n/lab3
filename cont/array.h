#pragma once

#include <cstddef>
#include <string>
#include <iosfwd>

class MyArray
{
public:
    // Конструктор (по умолчанию 8 элементов)
    explicit MyArray(std::size_t initialCapacity = 8);

    // Деструктор
    ~MyArray();

    // Rule of Five
    MyArray(const MyArray& other);                 // копирующий конструктор
    MyArray(MyArray&& other) noexcept;             // move-конструктор

    MyArray& operator=(const MyArray& other);      // копирующее присваивание
    MyArray& operator=(MyArray&& other) noexcept;  // move-присваивание

    // Базовые операции
    void pushBack(const std::string& value);
    void insert(std::size_t index, const std::string& value);
    void removeAt(std::size_t index);

    std::string& at(std::size_t index);
    [[nodiscard]] const std::string& at(std::size_t index) const;

    void set(std::size_t index, const std::string& value);

    // Операторы индексации (без проверки границ)
    std::string& operator[](std::size_t index) noexcept;
    const std::string& operator[](std::size_t index) const noexcept;

    // Вспомогательные методы
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t getCapacity() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    void print() const;
    void resize(std::size_t newCapacity);

    // ТЕКСТОВАЯ СЕРИАЛИЗАЦИЯ (в строку) 
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& dataString);

    // ТЕКСТОВАЯ СЕРИАЛИЗАЦИЯ ЧЕРЕЗ ПОТОК 
    void serializeText(std::ostream& outputStream) const;
    void deserializeText(std::istream& inputStream);

    // bИНАРНАЯ СЕРИАЛИЗАЦИЯ 
    void serializeBinary(std::ostream& outputStream) const;
    void deserializeBinary(std::istream& inputStream);

    // Обмен содержимым
    void swap(MyArray& other) noexcept;

private:
    std::string* dataPtr;   // указатель на массив строк
    std::size_t capacity;   // вместимость
    std::size_t length;     // текущее количество элементов
};
