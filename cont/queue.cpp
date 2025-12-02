#include "queue.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

// конструкторы / деструктор

Queue::Queue() noexcept = default;

Queue::~Queue()
{
    clear();
}

void Queue::clear() noexcept
{
    Node* current = frontNode;
    while (current != nullptr) {
        Node* nodeToDelete = current;
        current            = current->next;
        delete nodeToDelete;
    }
    frontNode = nullptr;
    backNode  = nullptr;
    sizeValue = 0;
}

// Rule of Five

Queue::Queue(const Queue& other)
    : frontNode(nullptr), //NOLINT
      backNode(nullptr), //NOLINT
      sizeValue(0) //NOLINT
{
    Node* current = other.frontNode;
    while (current != nullptr) {
        push(current->value);
        current = current->next;
    }
}

Queue::Queue(Queue&& other) noexcept
    : frontNode(other.frontNode),
      backNode(other.backNode),
      sizeValue(other.sizeValue)
{
    other.frontNode = nullptr;
    other.backNode  = nullptr;
    other.sizeValue = 0;
}

Queue& Queue::operator=(const Queue& other)
{
    if (this == &other) {
        return *this;
    }

    Queue tmp(other);
    swap(tmp);
    return *this;
}

Queue& Queue::operator=(Queue&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    clear();
    frontNode = other.frontNode;
    backNode  = other.backNode;
    sizeValue = other.sizeValue;

    other.frontNode = nullptr;
    other.backNode  = nullptr;
    other.sizeValue = 0;

    return *this;
}

void Queue::swap(Queue& other) noexcept
{
    using std::swap;
    swap(frontNode, other.frontNode);
    swap(backNode, other.backNode);
    swap(sizeValue, other.sizeValue);
}

// базовые операции

void Queue::push(const std::string& value)
{
    Node* node = new Node(value);
    if (backNode != nullptr) {
        backNode->next = node;
    } else {
        frontNode = node;
    }
    backNode = node;
    ++sizeValue;
}

std::string Queue::pop()
{
    if (frontNode == nullptr) {
        throw std::out_of_range("Queue::pop: queue is empty");
    }

    std::string result = frontNode->value;
    Node* nodeToDelete = frontNode;
    frontNode          = frontNode->next;
    if (frontNode == nullptr) {
        backNode = nullptr;
    }
    delete nodeToDelete;
    --sizeValue;
    return result;
}

const std::string& Queue::front() const
{
    if (frontNode == nullptr) {
        throw std::out_of_range("Queue::front: queue is empty");
    }
    return frontNode->value;
}

const std::string& Queue::back() const
{
    if (backNode == nullptr) {
        throw std::out_of_range("Queue::back: queue is empty");
    }
    return backNode->value;
}

std::size_t Queue::size() const noexcept
{
    return sizeValue;
}

bool Queue::empty() const noexcept
{
    return sizeValue == 0;
}

void Queue::print() const
{
    std::cout << "[";
    Node* current = frontNode;
    while (current != nullptr) {
        std::cout << current->value;
        if (current->next != nullptr) {
            std::cout << ", ";
        }
        current = current->next;
    }
    std::cout << "]\n";
}

// текстовая сериализация

void Queue::serializeText(std::ostream& outputStream) const
{
    Node* current = frontNode;
    while (current != nullptr) {
        outputStream << current->value << '\n';
        current = current->next;
    }
}

void Queue::deserializeText(std::istream& inputStream)
{
    clear();
    std::string line;
    while (std::getline(inputStream, line)) {
        if (!line.empty()) {
            push(line);
        }
    }
}

std::string Queue::serialize() const
{
    std::ostringstream output;
    serializeText(output);
    return output.str();
}

void Queue::deserialize(const std::string& textData)
{
    std::istringstream input(textData);
    deserializeText(input);
}

// бинарная сериализация

void Queue::serializeBinary(std::ostream& outputStream) const
{
    std::uint64_t count = static_cast<std::uint64_t>(sizeValue);
    outputStream.write(reinterpret_cast<const char*>(&count), sizeof(count));

    Node* current = frontNode;
    while (current != nullptr) {
        std::uint64_t length =
            static_cast<std::uint64_t>(current->value.size());
        outputStream.write(reinterpret_cast<const char*>(&length),
                           sizeof(length));
        if (length > 0) {
            outputStream.write(current->value.data(),
                               static_cast<std::streamsize>(length));
        }
        current = current->next;
    }

    if (!outputStream) {
        throw std::runtime_error("Queue::serializeBinary: ERROR");
    }
}

void Queue::deserializeBinary(std::istream& inputStream)
{
    clear();

    std::uint64_t count = 0;
    inputStream.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!inputStream) {
        throw std::runtime_error(
            "Queue::deserializeBinary: ERROR");
    }

    for (std::uint64_t i = 0; i < count; ++i) {
        std::uint64_t length = 0;
        inputStream.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (!inputStream) {
            throw std::runtime_error(
                "Queue::deserializeBinary: ERROR");
        }

        std::string value;
        value.resize(static_cast<std::size_t>(length));
        if (length > 0) {
            inputStream.read(value.data(),
                             static_cast<std::streamsize>(length));
            if (!inputStream) {
                throw std::runtime_error(
                    "Queue::deserializeBinary: ERROR");
            }
        }
        push(value);
    }
}
