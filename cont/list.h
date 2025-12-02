#pragma once

#include <iosfwd>
#include <string>
#include <utility>

class List; // forward

class LNode
{
public:
    explicit LNode(std::string valueIn) noexcept
        : value(std::move(valueIn))
    {
    }

    [[nodiscard]] const std::string& getValue() const noexcept
    {
        return value;
    }

    [[nodiscard]] LNode* getNext() const noexcept
    {
        return next;
    }

    [[nodiscard]] LNode* getPrev() const noexcept
    {
        return prev;
    }

private:
    std::string value;
    LNode*      next{nullptr};
    LNode*      prev{nullptr};

    friend class List;
};

class List
{
public:
    List() noexcept;
    ~List();

    List(const List&) = delete;
    List& operator=(const List&) = delete;
    List(List&&) = delete;
    List& operator=(List&&) = delete;

    void pushFront(const std::string& value);
    void pushBack(const std::string& value);
    void popFront();
    void popBack();
    void removeByValue(const std::string& value);
    [[nodiscard]] LNode* findNode(const std::string& value);
    void insertAfter(const std::string& afterValue, const std::string& newValue);
    void insertBefore(const std::string& beforeValue, const std::string& newValue);
    void removeAfter(const std::string& afterValue);
    void removeBefore(const std::string& beforeValue);
    void print() const;

    // текстовая сериализация
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& text);

    void serializeText(std::ostream& outputStream) const;
    void deserializeText(std::istream& inputStream);

    // бинарная сериализация
    void serializeBinary(std::ostream& outputStream) const;
    void deserializeBinary(std::istream& inputStream);

private:
    LNode* headNode{nullptr};
    LNode* tailNode{nullptr};

    void clear() noexcept;
};
