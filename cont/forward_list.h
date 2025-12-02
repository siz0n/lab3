#pragma once

#include <iosfwd>
#include <string>
#include <utility>

class ForwardList;  // forward-declare

class FNode
{
public:
    explicit FNode(std::string valueIn)
        : value(std::move(valueIn))
    {
    }

    [[nodiscard]] const std::string& getValue() const noexcept
    {
        return value;
    }

    [[nodiscard]] FNode* getNext() const noexcept
    {
        return next;
    }

private:
    std::string value;
    FNode* next = nullptr;   // default member initializer, то что просит clang-tidy

    friend class ForwardList;
};

class ForwardList
{
public:
    ForwardList() noexcept;
    ~ForwardList();

    ForwardList(const ForwardList&) = delete;
    ForwardList& operator=(const ForwardList&) = delete;
    ForwardList(ForwardList&&) = delete;
    ForwardList& operator=(ForwardList&&) = delete;

    void pushFront(const std::string& value);
    void pushBack(const std::string& value);
    void popFront();
    void popBack();
    void removeByValue(const std::string& value);
    [[nodiscard]] FNode* findNode(const std::string& value) const;
    void print() const;

    void insertAfter(const std::string& afterValue, const std::string& newValue);
    void insertBefore(const std::string& beforeValue, const std::string& newValue);
    void removeAfter(const std::string& afterValue);
    void removeBefore(const std::string& beforeValue);

    // --- текстовая сериализация ---
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& text);

    void serializeText(std::ostream& outputStream) const;
    void deserializeText(std::istream& inputStream);

    // --- бинарная сериализация ---
    void serializeBinary(std::ostream& outputStream) const;
    void deserializeBinary(std::istream& inputStream);

private:
    FNode* head = nullptr;   // тоже без подчёркивания, с дефолтной инициализацией

    void clear() noexcept;
};
