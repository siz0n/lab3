#pragma once

#include <cstddef>
#include <string>
#include <iosfwd>

class AvlTree
{
public:
    AvlTree();
    ~AvlTree();

    // Rule of Five
    AvlTree(const AvlTree& other);
    AvlTree(AvlTree&& other) noexcept;

    AvlTree& operator=(const AvlTree& other);
    AvlTree& operator=(AvlTree&& other) noexcept;

    // Базовые операции
    void insert(const std::string& value);
    void remove(const std::string& value);
    [[nodiscard]] bool contains(const std::string& value) const;

    // Вспомогательные методы
    void print() const;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    //  текстовая сериализация (префиксный обход, '#' для nullptr) 
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& data);

    //  бинарная сериализация 
    void serializeBinary(std::ostream& outputStream) const;
    void deserializeBinary(std::istream& inputStream);

    void swap(AvlTree& other) noexcept;

private:
    struct Node //NOLINT
    {
        std::string value; //NOLINT
        Node* left; //NOLINT 
        Node* right; //NOLINT 
        int height; //NOLINT

        explicit Node(const std::string& v) //NOLINT
            : value(v),
              left(nullptr),
              right(nullptr),
              height(1)
            {}; 
              
        
    };

    Node* root_; //NOLINT
    std::size_t size_; //NOLINT

    // Вспомогательные функции
    static int heightOf(Node* node) noexcept;
    static int balanceFactor(Node* node) noexcept;

    static Node* rotateRight(Node* parentNode);
    static Node* rotateLeft(Node* parentNode);

    static Node* insertNode(Node* node, const std::string& value, bool& inserted);
    static Node* removeNode(Node* node, const std::string& value, bool& removed);
    static bool containsNode(Node* node, const std::string& value);

    static void printRec(Node* node, int depth);

    static void serializeRec(std::ostringstream& oss, Node* node);
    static Node* deserializeRec(std::istringstream& iss);

    static void serializeBinaryRec(std::ostream& outputStream, Node* node);
    static Node* deserializeBinaryRec(std::istream& inputStream);

    static void clearSubtree(Node* node) noexcept;
    static Node* cloneSubtree(Node* node);
};
