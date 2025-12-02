#include "stack.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

//  базовые операции / управление памятью 

Stack::Stack() noexcept = default;

Stack::~Stack()
{
    clear();
}

void Stack::clear() noexcept
{
    StackNode* current = topNode;
    while (current != nullptr) {
        StackNode* nodeToDelete = current;
        current                 = current->next;
        delete nodeToDelete;
    }
    topNode = nullptr;
}

void Stack::push(const std::string& value)
{
    StackNode* newNode = new StackNode(value);
    newNode->next = topNode;  // новый → старый топ
    topNode = newNode;            
}

std::string Stack::pop()
{
    if (topNode == nullptr) {
        throw std::out_of_range("Stack::pop: empty stack");
    }

    std::string resultValue = topNode->value;
    StackNode* nodeToDelete = topNode;
    topNode                 = topNode->next;
    delete nodeToDelete;

    return resultValue;
}

void Stack::print() const
{
    StackNode* current = topNode;
    std::cout << '[';
    while (current != nullptr) {
        std::cout << current->value;
        if (current->next != nullptr) {
            std::cout << ", ";
        }
        current = current->next;
    }
    std::cout << "]\n";
}

bool Stack::empty() const noexcept
{
    return topNode == nullptr;
}

//  текстовая сериализация 

void Stack::serializeText(std::ostream& os) const {
    for (StackNode* n = topNode; n != nullptr; n = n->next)
        os << n->value << '\n';
}

void Stack::deserializeText(std::istream& input)
{
    clear();

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(input, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }


    for (int i = (int)lines.size() - 1; i >= 0; --i)
        push(lines[i]);
}





std::string Stack::serialize() const
{
    std::ostringstream output;
    serializeText(output);
    return output.str();
}

void Stack::deserialize(const std::string& text)
{
    std::istringstream input(text);
    deserializeText(input);
}

//  бинарная сериализация 

void Stack::serializeBinary(std::ostream& outputStream) const
{
    // сначала посчитаем количество
    std::uint64_t count = 0;
    StackNode* current  = topNode;
    while (current != nullptr) {
        ++count;
        current = current->next;
    }

    outputStream.write(reinterpret_cast<const char*>(&count), sizeof(count));

    current = topNode;
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
        throw std::runtime_error("Stack::serializeBinary: ERROR");
    }
}

void Stack::deserializeBinary(std::istream& inputStream)
{
    clear();

    std::uint64_t count = 0;
    inputStream.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!inputStream) {
        throw std::runtime_error(
            "Stack::deserializeBinary: ERROR");
    }

    std::vector<std::string> lines;
    lines.reserve(static_cast<std::size_t>(count));

    for (std::uint64_t index = 0; index < count; ++index) {
        std::uint64_t length = 0;
        inputStream.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (!inputStream) {
            throw std::runtime_error(
                "Stack::deserializeBinary: ERROR");
        }

        std::string value;
        value.resize(static_cast<std::size_t>(length));
        if (length > 0) {
            inputStream.read(value.data(),
                             static_cast<std::streamsize>(length));
            if (!inputStream) {
                throw std::runtime_error(
                    "Stack::deserializeBinary: ERROR");
            }
        }
        lines.push_back(std::move(value));
    }

    // восстановим порядок: последний прочитанный элемент должен оказаться внизу стека
    for (std::size_t index = lines.size(); index > 0; --index) {
        push(lines[index - 1]);
    }
}
