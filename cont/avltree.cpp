#include "avltree.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cstdint>

//  конструктор / деструктор 

AvlTree::AvlTree()
    : root_(nullptr),
      size_(0)
{
}

AvlTree::~AvlTree()
{
    clearSubtree(root_);
    root_ = nullptr;
    size_ = 0;
}

//  вспомогательные функции очистки/клонирования 

void AvlTree::clearSubtree(Node* node) noexcept
{
    if (node == nullptr) {
        return;
    }
    clearSubtree(node->left);
    clearSubtree(node->right);
    delete node;
}

AvlTree::Node* AvlTree::cloneSubtree(Node* node)
{
    if (node == nullptr) {
        return nullptr;
    }

    Node* newNode = new Node(node->value);
    newNode->height = node->height;
    newNode->left = cloneSubtree(node->left);
    newNode->right = cloneSubtree(node->right);
    return newNode;
}

//  высота / баланс 

int AvlTree::heightOf(Node* node) noexcept
{
    return node != nullptr ? node->height : 0;
}

int AvlTree::balanceFactor(Node* node) noexcept
{
    return node != nullptr ? heightOf(node->left) - heightOf(node->right) : 0;
}

//  повороты 

AvlTree::Node* AvlTree::rotateRight(Node* parentNode)
{
    Node* leftChild = parentNode->left;
    Node* transferSubtree = leftChild->right;

    leftChild->right = parentNode;
    parentNode->left = transferSubtree;

    parentNode->height = std::max(heightOf(parentNode->left), heightOf(parentNode->right)) + 1;
    leftChild->height = std::max(heightOf(leftChild->left), heightOf(leftChild->right)) + 1;

    return leftChild;
}

AvlTree::Node* AvlTree::rotateLeft(Node* parentNode)
{
    Node* rightChild= parentNode->right;
    Node* transferSubtree = rightChild->left;

    rightChild->left = parentNode;
    parentNode->right = transferSubtree;

    parentNode->height = std::max(heightOf(parentNode->left), heightOf(parentNode->right)) + 1;
    rightChild->height = std::max(heightOf(rightChild->left), heightOf(rightChild->right)) + 1;

    return rightChild;
}

//  вставка 

AvlTree::Node* AvlTree::insertNode(Node* node, const std::string& value, bool& inserted)
{
    if (node == nullptr) {
        inserted = true;
        return new Node(value);
    }

    if (value < node->value) {
        node->left = insertNode(node->left, value, inserted);
    } else if (value > node->value) {
        node->right = insertNode(node->right, value, inserted);
    } else {
        inserted = false;
        return node;
    }

    node->height = 1 + std::max(heightOf(node->left), heightOf(node->right));
    const int balance = balanceFactor(node);

    // LL
    if (balance > 1 && value < node->left->value) {
        return rotateRight(node);
    }

    // RR
    if (balance < -1 && value > node->right->value) {
        return rotateLeft(node);
    }

    // LR
    if (balance > 1 && value > node->left->value) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    // RL
    if (balance < -1 && value < node->right->value) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

void AvlTree::insert(const std::string& value)
{
    bool inserted = false;
    root_ = insertNode(root_, value, inserted);
    if (inserted) {
        ++size_;
    }
}

//  удаление 

AvlTree::Node* AvlTree::removeNode(Node* node, const std::string& value, bool& removed)
{
    if (node == nullptr) {
        return nullptr;
    }

    if (value < node->value) {
        node->left = removeNode(node->left, value, removed);
    } else if (value > node->value) {
        node->right = removeNode(node->right, value, removed);
    } else {
        // нашли узел
        removed = true;

        // 0 или 1 ребёнок
        if (node->left == nullptr || node->right == nullptr) {
            Node* child = (node->left != nullptr) ? node->left : node->right;

            if (child == nullptr) {
                // вообще нет детей
                delete node;
                return nullptr;
            }

            // есть ровно один ребёнок — просто поднимаем его наверх
            Node* toDelete = node;
            node = child;
            delete toDelete;
        } else {
            // два ребёнка — берём inorder-преемника (минимум в правом поддереве)
            Node* succ = node->right;
            while (succ->left != nullptr) {
                succ = succ->left;
            }

            node->value = succ->value;
            // удаляем преемника из правого поддерева
            node->right = removeNode(node->right, succ->value, removed);
        }
    }

    // если после удаления поддерева здесь больше нет узла
    if (node == nullptr) {
        return nullptr;
    }

    // обновляем высоту
    node->height = 1 + std::max(heightOf(node->left), heightOf(node->right));
    const int balance = balanceFactor(node);

    // LL
    if (balance > 1 && balanceFactor(node->left) >= 0) {
        return rotateRight(node);
    }

    // LR
    if (balance > 1 && balanceFactor(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    // RR
    if (balance < -1 && balanceFactor(node->right) <= 0) {
        return rotateLeft(node);
    }

    // RL
    if (balance < -1 && balanceFactor(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}


void AvlTree::remove(const std::string& value)
{
    bool removed = false;
    root_ = removeNode(root_, value, removed);
    if (removed && size_ > 0U) {
        --size_;
    }
}


//  поиск 

bool AvlTree::containsNode(Node* node, const std::string& value)
{
    if (node == nullptr) {
        return false;
    }
    if (value == node->value) {
        return true;
    }
    if (value < node->value) {
        return containsNode(node->left, value);
    }
    return containsNode(node->right, value);
}

bool AvlTree::contains(const std::string& value) const
{
    return containsNode(root_, value);
}

//  печать 

void AvlTree::printRec(Node* node, int depth)
{
    if (node == nullptr) {
        return;
    }

    printRec(node->right, depth + 1);
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }
    std::cout << node->value << "\n";
    printRec(node->left, depth + 1);
}

void AvlTree::print() const
{
    printRec(root_, 0);
}

//  размер / пустота 

std::size_t AvlTree::size() const noexcept
{
    return size_;
}

bool AvlTree::empty() const noexcept
{
    return size_ == 0;
}

//  текстовая сериализация 

void AvlTree::serializeRec(std::ostringstream& oss, Node* node)
{
    if (node == nullptr) {
        oss << "#\n";
        return;
    }

    oss << node->value << "\n";
    serializeRec(oss, node->left);
    serializeRec(oss, node->right);
}

std::string AvlTree::serialize() const
{
    std::ostringstream oss;
    serializeRec(oss, root_);
    return oss.str();
}

AvlTree::Node* AvlTree::deserializeRec(std::istringstream& iss) //NOLINT
{
    std::string line;
    if (!std::getline(iss, line)) {
        return nullptr;
    }

    if (line == "#") {
        return nullptr;
    }

    Node* node = new Node(line);
    node->left = deserializeRec(iss);
    node->right = deserializeRec(iss);
    node->height = 1 + std::max(heightOf(node->left), heightOf(node->right));
    return node;
}

void AvlTree::deserialize(const std::string& data)
{
    clearSubtree(root_);
    root_ = nullptr;
    size_ = 0;

    std::istringstream iss(data);
    root_ = deserializeRec(iss);

    // пересчёт size_
    std::size_t count = 0;
    std::vector<Node*> stack;
    if (root_ != nullptr) {
        stack.push_back(root_);
    }
    while (!stack.empty()) {
        Node* node = stack.back();
        stack.pop_back();
        ++count;
        if (node->left != nullptr) {
            stack.push_back(node->left);
        }
        if (node->right != nullptr) {
            stack.push_back(node->right);
        }
    }
    size_ = count;
}

//  бинарная сериализация 

void AvlTree::serializeBinaryRec(std::ostream& outputStream, Node* node)
{
    std::uint8_t flag = (node != nullptr) ? 1 : 0;
    outputStream.write(reinterpret_cast<const char*>(&flag), sizeof(flag));
    if (node == nullptr) {
        return;
    }

    std::uint64_t len = static_cast<std::uint64_t>(node->value.size());
    outputStream.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        outputStream.write(node->value.data(), static_cast<std::streamsize>(len));
    }

    serializeBinaryRec(outputStream, node->left);
    serializeBinaryRec(outputStream, node->right);
}

void AvlTree::serializeBinary(std::ostream& outputStream) const
{
    serializeBinaryRec(outputStream, root_);
    if (!outputStream) {
        throw std::runtime_error("AvlTree::serializeBinary: error");
    }
}

AvlTree::Node* AvlTree::deserializeBinaryRec(std::istream& inputStream)
{
    std::uint8_t flag = 0;
    inputStream.read(reinterpret_cast<char*>(&flag), sizeof(flag));
    if (!inputStream) {
        throw std::runtime_error("AvlTree::deserializeBinaryRec: error");
    }

    if (flag == 0) {
        return nullptr;
    }

    std::uint64_t len = 0;
    inputStream.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (!inputStream) {
        throw std::runtime_error("AvlTree::deserializeBinaryRec: error");
    }

    std::string value;
    value.resize(static_cast<std::size_t>(len));
    if (len > 0) {
        inputStream.read(value.data(), static_cast<std::streamsize>(len));
        if (!inputStream) {
            throw std::runtime_error("AvlTree::deserializeBinaryRec: error");
        }
    }

    Node* node = new Node(value);
    node->left = deserializeBinaryRec(inputStream);
    node->right = deserializeBinaryRec(inputStream);
    node->height = 1 + std::max(heightOf(node->left), heightOf(node->right));
    return node;
}

void AvlTree::deserializeBinary(std::istream& inputStream)
{
    clearSubtree(root_);
    root_ = nullptr;
    size_ = 0;

    root_ = deserializeBinaryRec(inputStream);

    // пересчёт size_
    std::size_t count = 0;
    std::vector<Node*> stack;
    if (root_ != nullptr) {
        stack.push_back(root_);
    }
    while (!stack.empty()) {
        Node* node = stack.back();
        stack.pop_back();
        ++count;
        if (node->left != nullptr) {
            stack.push_back(node->left);
        }
        if (node->right != nullptr) {
            stack.push_back(node->right);
        }
    }
    size_ = count;
}

//  Rule of Five: копирование / перемещение 

AvlTree::AvlTree(const AvlTree& other)
    : root_(cloneSubtree(other.root_)),
      size_(other.size_)
{
    
}

AvlTree::AvlTree(AvlTree&& other) noexcept
    : root_(other.root_),
      size_(other.size_)
{
    other.root_ = nullptr;
    other.size_ = 0;
}

AvlTree& AvlTree::operator=(const AvlTree& other)
{
    if (this == &other) {
        return *this;
    }

    AvlTree tmp(other);
    swap(tmp);
    return *this;
}

AvlTree& AvlTree::operator=(AvlTree&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    clearSubtree(root_);
    root_ = other.root_;
    size_ = other.size_;

    other.root_ = nullptr;
    other.size_ = 0;

    return *this;
}

//  swap 

void AvlTree::swap(AvlTree& other) noexcept
{
    using std::swap;
    swap(root_, other.root_);
    swap(size_, other.size_);
}
