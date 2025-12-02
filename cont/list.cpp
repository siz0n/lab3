#include "list.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>

// ---------- базовые операции ----------

List::List() noexcept = default;

List::~List()
{
    clear();
}

void List::clear() noexcept
{
    LNode* current = headNode;
    while (current != nullptr) {
        LNode* nodeToDelete = current;
        current             = current->next;
        delete nodeToDelete;
    }
    headNode = nullptr;
    tailNode = nullptr;
}

void List::pushFront(const std::string& value)
{
    LNode* node = new LNode(value);
    node->next  = headNode;
    node->prev  = nullptr;

    if (headNode != nullptr) {
        headNode->prev = node;
    }
    headNode = node;

    if (tailNode == nullptr) {
        tailNode = node;
    }
}

void List::pushBack(const std::string& value)
{
    LNode* node = new LNode(value);
    node->prev  = tailNode;
    node->next  = nullptr;

    if (tailNode != nullptr) {
        tailNode->next = node;
    }
    tailNode = node;

    if (headNode == nullptr) {
        headNode = node;
    }
}

void List::popFront()
{
    if (headNode == nullptr) {
        return;
    }

    LNode* node = headNode;
    headNode    = headNode->next;

    if (headNode != nullptr) {
        headNode->prev = nullptr;
    } else {
        tailNode = nullptr;
    }

    delete node;
}

void List::popBack()
{
    if (tailNode == nullptr) {
        return;
    }

    LNode* node = tailNode;
    tailNode    = tailNode->prev;

    if (tailNode != nullptr) {
        tailNode->next = nullptr;
    } else {
        headNode = nullptr;
    }

    delete node;
}

void List::removeByValue(const std::string& value)
{
    LNode* current = headNode;
    while (current != nullptr) {
        if (current->value == value) {
            LNode* nodeToDelete = current;

            if (current->prev != nullptr) {
                current->prev->next = current->next;
            } else {
                headNode = current->next;
            }

            if (current->next != nullptr) {
                current->next->prev = current->prev;
            } else {
                tailNode = current->prev;
            }

            current = current->next;
            delete nodeToDelete;
        } else {
            current = current->next;
        }
    }
}

LNode* List::findNode(const std::string& value)
{
    LNode* current = headNode;
    while (current != nullptr) {
        if (current->value == value) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

void List::insertAfter(const std::string& afterValue, const std::string& newValue)
{
    LNode* current = headNode;
    while (current != nullptr) {
        if (current->value == afterValue) {
            LNode* node = new LNode(newValue);
            node->prev  = current;
            node->next  = current->next;

            if (current->next != nullptr) {
                current->next->prev = node;
            }
            current->next = node;

            if (current == tailNode) {
                tailNode = node;
            }
            return;
        }
        current = current->next;
    }
}

void List::insertBefore(const std::string& beforeValue, const std::string& newValue)
{
    LNode* current = headNode;
    while (current != nullptr) {
        if (current->value == beforeValue) {
            LNode* node = new LNode(newValue);
            node->next  = current;
            node->prev  = current->prev;

            if (current->prev != nullptr) {
                current->prev->next = node;
            }
            current->prev = node;

            if (current == headNode) {
                headNode = node;
            }
            return;
        }
        current = current->next;
    }
}

void List::removeAfter(const std::string& afterValue)
{
    LNode* current = headNode;
    while (current != nullptr && current->next != nullptr) {
        if (current->value == afterValue) {
            LNode* nodeToDelete = current->next;
            current->next       = nodeToDelete->next;

            if (nodeToDelete->next != nullptr) {
                nodeToDelete->next->prev = current;
            }
            if (nodeToDelete == tailNode) {
                tailNode = current;
            }

            delete nodeToDelete;
            return;
        }
        current = current->next;
    }
}

void List::removeBefore(const std::string& beforeValue)
{
    LNode* current = headNode;
    while (current != nullptr) {
        if (current->value == beforeValue && current->prev != nullptr) {
            LNode* nodeToDelete = current->prev;
            current->prev       = nodeToDelete->prev;

            if (nodeToDelete->prev != nullptr) {
                nodeToDelete->prev->next = current;
            }
            if (nodeToDelete == headNode) {
                headNode = current;
            }

            delete nodeToDelete;
            return;
        }
        current = current->next;
    }
}

void List::print() const
{
    LNode* current = headNode;
    std::cout << '[';
    while (current != nullptr) {
        std::cout << current->value;
        if (current->next != nullptr) {
            std::cout << "<->";
        }
        current = current->next;
    }
    std::cout << "]\n";
}

// ---------- текстовая сериализация ----------

void List::serializeText(std::ostream& outputStream) const
{
    LNode* current = headNode;
    while (current != nullptr) {
        outputStream << current->value << '\n';
        current       = current->next;
    }
}

void List::deserializeText(std::istream& inputStream)
{
    clear();

    std::string line;
    while (std::getline(inputStream, line)) {
        if (!line.empty()) {
            pushBack(line);
        }
    }
}

std::string List::serialize() const
{
    std::ostringstream output;
    serializeText(output);
    return output.str();
}

void List::deserialize(const std::string& text)
{
    std::istringstream input(text);
    deserializeText(input);
}

// ---------- бинарная сериализация ----------

void List::serializeBinary(std::ostream& outputStream) const
{
    std::uint64_t count = 0;
    LNode* current      = headNode;
    while (current != nullptr) {
        ++count;
        current = current->next;
    }

    outputStream.write(reinterpret_cast<const char*>(&count), sizeof(count));

    current = headNode;
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
        throw std::runtime_error("List::serializeBinary: write error");
    }
}

void List::deserializeBinary(std::istream& inputStream)
{
    clear();

    std::uint64_t count = 0;
    inputStream.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!inputStream) {
        throw std::runtime_error(
            "List::deserializeBinary: cannot read count");
    }

    for (std::uint64_t i = 0; i < count; ++i) {
        std::uint64_t length = 0;
        inputStream.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (!inputStream) {
            throw std::runtime_error(
                "List::deserializeBinary: cannot read string size");
        }

        std::string value;
        value.resize(static_cast<std::size_t>(length));
        if (length > 0) {
            inputStream.read(value.data(),
                             static_cast<std::streamsize>(length));
            if (!inputStream) {
                throw std::runtime_error(
                    "List::deserializeBinary: cannot read string data");
            }
        }
        pushBack(value);
    }
}
