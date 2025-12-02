#pragma once

#include <iosfwd>
#include <string>
#include <utility>

class Stack;  

class StackNode
{
public:
    explicit StackNode(std::string valueIn) noexcept
        : value(std::move(valueIn))
    {
    }

private:
    std::string value;
    StackNode*  next{nullptr};

    friend class Stack;
};

class Stack
{
public:
    Stack() noexcept;
    ~Stack();

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    Stack(Stack&&) = delete;
    Stack& operator=(Stack&&) = delete;

    void push(const std::string& value);  // наверх стека
    std::string pop();                    // std::out_of_range, если пуст

    void print() const;

    [[nodiscard]] bool empty() const noexcept;

    // текстовая сериализация
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& text);

    void serializeText(std::ostream& outputStream) const;
    void deserializeText(std::istream& inputStream);

    // бинарная сериализация
    void serializeBinary(std::ostream& outputStream) const;
    void deserializeBinary(std::istream& inputStream);

private:
    StackNode* topNode{nullptr};

    void clear() noexcept;
};
