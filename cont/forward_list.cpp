#include "forward_list.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>

// ---------- базовые операции ----------

ForwardList::ForwardList() noexcept = default;

ForwardList::~ForwardList()
{
    clear();
}

void ForwardList::clear() noexcept
{
    while (head != nullptr) {
        FNode* node = head;
        head = head->next;
        delete node;
    }
}

void ForwardList::pushFront(const std::string& value)
{
    FNode* node = new FNode(value);
    node->next = head;
    head = node;
}

void ForwardList::pushBack(const std::string& value)
{
    FNode* node = new FNode(value);
    if (head == nullptr) {
        head = node;
        return;
    }

    FNode* current = head;
    while (current->next != nullptr) {
        current = current->next;
    }
    current->next = node;
}

void ForwardList::popFront()
{
    if (head == nullptr) {
        return;
    }
    FNode* node = head;
    head = head->next;
    delete node;
}

void ForwardList::popBack()
{
    if (head == nullptr) {
        return;
    }
    if (head->next == nullptr) {
        delete head;
        head = nullptr;
        return;
    }

    FNode* current = head;
    while (current->next != nullptr && current->next->next != nullptr) {
        current = current->next;
    }
    delete current->next;
    current->next = nullptr;
}

void ForwardList::removeByValue(const std::string& value)
{
    // убираем совпадения в начале
    while (head != nullptr && head->value == value) {
        popFront();
    }

    FNode* current = head;
    while (current != nullptr && current->next != nullptr) {
        if (current->next->value == value) {
            FNode* node = current->next;
            current->next = node->next;
            delete node;
        } else {
            current = current->next;
        }
    }
}

FNode* ForwardList::findNode(const std::string& value) const
{
    FNode* current = head;
    while (current != nullptr) {
        if (current->value == value) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

void ForwardList::insertAfter(const std::string& afterValue, const std::string& newValue)
{
    FNode* current = head;
    while (current != nullptr) {
        if (current->value == afterValue) {
            FNode* node = new FNode(newValue);
            node->next = current->next;
            current->next = node;
            return;
        }
        current = current->next;
    }
}

void ForwardList::insertBefore(const std::string& beforeValue, const std::string& newValue)
{
    if (head == nullptr) {
        return;
    }
    if (head->value == beforeValue) {
        pushFront(newValue);
        return;
    }

    FNode* previous = head;
    FNode* current = head->next;
    while (current != nullptr) {
        if (current->value == beforeValue) {
            FNode* node = new FNode(newValue);
            previous->next = node;
            node->next = current;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void ForwardList::removeAfter(const std::string& afterValue)
{
    FNode* current = head;
    while (current != nullptr && current->next != nullptr) {
        if (current->value == afterValue) {
            FNode* node = current->next;
            current->next = node->next;
            delete node;
            return;
        }
        current = current->next;
    }
}

void ForwardList::removeBefore(const std::string& beforeValue)
{
    if (head == nullptr || head->next == nullptr) {
        return;
    }
    if (head->next->value == beforeValue) {
        popFront();
        return;
    }

    FNode* prevPrev = head;
    FNode* prev = head->next;
    FNode* current = prev->next;
    while (current != nullptr) {
        if (current->value == beforeValue) {
            FNode* node = prev;
            prevPrev->next = current;
            delete node;
            return;
        }
        prevPrev = prev;
        prev = current;
        current = current->next;
    }
}

void ForwardList::print() const
{
    FNode* current = head;
    std::cout << '[';
    while (current != nullptr) {
        std::cout << current->value;
        if (current->next != nullptr) {
            std::cout << "->";
        }
        current = current->next;
    }
    std::cout << "]\n";
}

// ---------- текстовая сериализация ----------

void ForwardList::serializeText(std::ostream& outputStream) const
{
    FNode* current = head;
    while (current != nullptr) {
        outputStream << current->value << '\n';
        current = current->next;
    }
}

void ForwardList::deserializeText(std::istream& inputStream)
{
    clear();

    std::string line;
    while (std::getline(inputStream, line)) {
        if (!line.empty()) {
            pushBack(line);
        }
    }
}

std::string ForwardList::serialize() const
{
    std::ostringstream output;
    serializeText(output);
    return output.str();
}

void ForwardList::deserialize(const std::string& text)
{
    std::istringstream input(text);
    deserializeText(input);
}

// ---------- бинарная сериализация ----------

void ForwardList::serializeBinary(std::ostream& outputStream) const
{
    // сначала считаем количество элементов
    std::uint64_t count = 0;
    FNode* current = head;
    while (current != nullptr) {
        ++count;
        current = current->next;
    }

    outputStream.write(reinterpret_cast<const char*>(&count), sizeof(count));

    current = head;
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
        throw std::runtime_error("ForwardList::serializeBinary: write error");
    }
}

void ForwardList::deserializeBinary(std::istream& inputStream)
{
    clear();

    std::uint64_t count = 0;
    inputStream.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!inputStream) {
        throw std::runtime_error(
            "ForwardList::deserializeBinary: cannot read count");
    }

    for (std::uint64_t i = 0; i < count; ++i) {
        std::uint64_t length = 0;
        inputStream.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (!inputStream) {
            throw std::runtime_error(
                "ForwardList::deserializeBinary: cannot read string size");
        }

        std::string value;
        value.resize(static_cast<std::size_t>(length));
        if (length > 0) {
            inputStream.read(value.data(),
                             static_cast<std::streamsize>(length));
            if (!inputStream) {
                throw std::runtime_error(
                    "ForwardList::deserializeBinary: cannot read string data");
            }
        }
        pushBack(value);
    }
}
