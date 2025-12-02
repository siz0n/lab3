#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <utility>

class Queue
{
public:
    Queue() noexcept;
    ~Queue();

    Queue(const Queue& other);
    Queue(Queue&& other) noexcept;

    Queue& operator=(const Queue& other);
    Queue& operator=(Queue&& other) noexcept;

    // Базовые операции
    void push(const std::string& value);        // в конец
    std::string pop();                          // из начала, std::out_of_range если пусто

    [[nodiscard]] const std::string& front() const;  
    [[nodiscard]] const std::string& back() const;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    void print() const;

    //  текстовая сериализация 
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& textData);

    void serializeText(std::ostream& outputStream) const;
    void deserializeText(std::istream& inputStream);

    //  бинарная сериализация 
    void serializeBinary(std::ostream& outputStream) const;
    void deserializeBinary(std::istream& inputStream);

    void swap(Queue& other) noexcept;

private:
    class Node
    {
    public:
        explicit Node(std::string valueIn) noexcept
            : value(std::move(valueIn))
        {
        }

    private:
        std::string value;
        Node* next{nullptr};   

        friend class Queue;
    };

    Node*       frontNode{nullptr};
    Node*       backNode{nullptr};
    std::size_t sizeValue{0};

    void clear() noexcept;
};
