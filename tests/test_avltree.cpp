
#include "catch_amalgamated.hpp"
#include "avltree.h"

#include <sstream>
#include <string>
#include <cstdint>
#include <iostream>




TEST_CASE("AvlTree: конструктор по умолчанию, пустота и размер", "[AvlTree]")
{
    AvlTree tree;
    REQUIRE(tree.empty());
    REQUIRE(tree.size() == 0);

    // Печать пустого дерева для покрытия print/printRec(nullptr)
    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    tree.print();
    std::cout.rdbuf(oldBuf);
}


// 2. ВСТАВКА И ПОИСК


TEST_CASE("AvlTree: простая вставка и contains", "[AvlTree]")
{
    AvlTree tree;
    tree.insert("b");
    tree.insert("a");
    tree.insert("c");

    REQUIRE_FALSE(tree.empty());
    REQUIRE(tree.size() == 3);
    REQUIRE(tree.contains("a"));
    REQUIRE(tree.contains("b"));
    REQUIRE(tree.contains("c"));
    REQUIRE_FALSE(tree.contains("x"));

    // Печать непустого дерева
    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    tree.print();
    std::cout.rdbuf(oldBuf);
}

TEST_CASE("AvlTree: повторная вставка не добавляет дубликат", "[AvlTree]")
{
    AvlTree tree;
    tree.insert("a");
    tree.insert("b");
    tree.insert("a"); // дубликат

    REQUIRE(tree.size() == 2U);
    REQUIRE(tree.contains("a"));
    REQUIRE(tree.contains("b"));
}


// 3. БАЛАНСИРОВКА / ПОВОРОТЫ (ОБЩАЯ ПРОВЕРКА)


TEST_CASE("AvlTree: вставки вызывают балансировку дерева", "[AvlTree]")
{
    SECTION("Последовательность, похожая на LL-поворот")
    {
        AvlTree tree;
        tree.insert("30");
        tree.insert("20");
        tree.insert("10");

        REQUIRE(tree.size() == 3);
        REQUIRE(tree.contains("30"));
        REQUIRE(tree.contains("20"));
        REQUIRE(tree.contains("10"));
    }

    SECTION("Последовательность, похожая на RR-поворот")
    {
        AvlTree tree;
        tree.insert("10");
        tree.insert("20");
        tree.insert("30");

        REQUIRE(tree.size() == 3);
        REQUIRE(tree.contains("10"));
        REQUIRE(tree.contains("20"));
        REQUIRE(tree.contains("30"));
    }

    SECTION("Смешанный случай (LR/RL)")
    {
        AvlTree tree;
        tree.insert("30");
        tree.insert("10");
        tree.insert("20");

        REQUIRE(tree.size() == 3);
        REQUIRE(tree.contains("10"));
        REQUIRE(tree.contains("20"));
        REQUIRE(tree.contains("30"));
    }
}


// 4. УДАЛЕНИЕ ЭЛЕМЕНТОВ


TEST_CASE("AvlTree: удаление листа, корня и несуществующего элемента", "[AvlTree]")
{
    AvlTree tree;
    tree.insert("b");
    tree.insert("a");
    tree.insert("c");
    tree.insert("d");

    REQUIRE(tree.size() == 4);

    SECTION("удаление листа")
    {
        tree.remove("d");
        REQUIRE(tree.size() == 3);
        REQUIRE_FALSE(tree.contains("d"));
    }

    SECTION("удаление корня с двумя детьми")
    {
        tree.remove("b");
        REQUIRE(tree.size() == 3);
        REQUIRE_FALSE(tree.contains("b"));
    }

    SECTION("удаление несуществующего элемента")
    {
        std::size_t oldSize = tree.size();
        tree.remove("zzz");
        REQUIRE(tree.size() == oldSize);
        REQUIRE_FALSE(tree.contains("zzz"));
    }
}


// 5. ПРАВИЛО ПЯТИ: КОПИЯ / ПЕРЕМЕЩЕНИЕ / SWAP


TEST_CASE("AvlTree: копирующий конструктор создаёт независимую копию", "[AvlTree]")
{
    AvlTree original;
    original.insert("b");
    original.insert("a");
    original.insert("c");

    AvlTree copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.contains("a"));
    REQUIRE(copy.contains("b"));
    REQUIRE(copy.contains("c"));

    original.insert("d");
    REQUIRE(original.size() == 4);
    REQUIRE(copy.size() == 3);
    REQUIRE_FALSE(copy.contains("d"));
}

TEST_CASE("AvlTree: копирующее присваивание и самоприсваивание", "[AvlTree]")
{
    AvlTree a;
    a.insert("1");
    a.insert("2");

    AvlTree b;
    b.insert("x");
    b.insert("y");
    b.insert("z");

    b = a;
    REQUIRE(b.size() == a.size());
    REQUIRE(b.contains("1"));
    REQUIRE(b.contains("2"));
    REQUIRE_FALSE(b.contains("x"));

    // самоприсваивание
    b = b;
    REQUIRE(b.size() == 2);
    REQUIRE(b.contains("1"));
    REQUIRE(b.contains("2"));
}

TEST_CASE("AvlTree: перемещающий конструктор и перемещающее присваивание", "[AvlTree]")
{
    AvlTree source;
    source.insert("a");
    source.insert("b");
    source.insert("c");

    std::size_t oldSize = source.size();

    AvlTree moved(std::move(source));
    REQUIRE(moved.size() == oldSize);
    REQUIRE(moved.contains("a"));
    REQUIRE(moved.contains("b"));
    REQUIRE(moved.contains("c"));
    REQUIRE(source.size() == 0);
    REQUIRE(source.empty());

    AvlTree target;
    target.insert("x");
    target = std::move(moved);

    REQUIRE(target.size() == oldSize);
    REQUIRE(target.contains("a"));
    REQUIRE(target.contains("b"));
    REQUIRE(target.contains("c"));
}

TEST_CASE("AvlTree: swap обменивает содержимое деревьев", "[AvlTree]")
{
    AvlTree a;
    a.insert("a1");
    a.insert("a2");

    AvlTree b;
    b.insert("b1");

    std::size_t aSize = a.size();
    std::size_t bSize = b.size();

    a.swap(b);

    REQUIRE(a.size() == bSize);
    REQUIRE(a.contains("b1"));
    REQUIRE_FALSE(a.contains("a1"));

    REQUIRE(b.size() == aSize);
    REQUIRE(b.contains("a1"));
    REQUIRE(b.contains("a2"));
}


// 6. ТЕКСТОВАЯ СЕРИАЛИЗАЦИЯ


TEST_CASE("AvlTree: текстовая serialize/deserialize возвращает то же множество ключей", "[AvlTree]")
{
    AvlTree original;
    original.insert("b");
    original.insert("a");
    original.insert("c");
    original.insert("d");

    std::string data = original.serialize();

    AvlTree restored;
    restored.deserialize(data);

    REQUIRE(restored.size() == original.size());
    REQUIRE(restored.contains("a"));
    REQUIRE(restored.contains("b"));
    REQUIRE(restored.contains("c"));
    REQUIRE(restored.contains("d"));
}

TEST_CASE("AvlTree: deserialize очищает старое дерево и поддерживает пустую строку", "[AvlTree]")
{
    AvlTree tree;
    tree.insert("x");
    tree.insert("y");
    REQUIRE(tree.size() == 2);

    SECTION("deserialize с пустой строкой -> пустое дерево")
    {
        tree.deserialize("");
        REQUIRE(tree.empty());
        REQUIRE(tree.size() == 0);
    }

    SECTION("deserialize перезаписывает дерево новыми данными")
    {
        AvlTree other;
        other.insert("a");
        other.insert("b");

        std::string data = other.serialize();
        tree.deserialize(data);

        REQUIRE(tree.size() == other.size());
        REQUIRE(tree.contains("a"));
        REQUIRE(tree.contains("b"));
        REQUIRE_FALSE(tree.contains("x"));
        REQUIRE_FALSE(tree.contains("y"));
    }
}


// 7. БИНАРНАЯ СЕРИАЛИЗАЦИЯ (ПОЗИТИВНЫЙ КЕЙС + ПАРА ОШИБОК)


TEST_CASE("AvlTree: бинарная serialize/deserialize сохраняет дерево", "[AvlTree]")
{
    AvlTree original;
    original.insert("m");
    original.insert("a");
    original.insert("z");
    original.insert("k");
    original.insert("русский текст");

    std::ostringstream oss(std::ios::binary);
    original.serializeBinary(oss);
    std::string bin = oss.str();

    std::istringstream iss(bin, std::ios::binary);
    AvlTree restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.size() == original.size());
    REQUIRE(restored.contains("m"));
    REQUIRE(restored.contains("a"));
    REQUIRE(restored.contains("z"));
    REQUIRE(restored.contains("k"));
    REQUIRE(restored.contains("русский текст"));
}

TEST_CASE("AvlTree: deserializeBinary выбрасывает, если не удаётся прочитать флаг", "[AvlTree]")
{
    std::string empty;
    std::istringstream iss(empty, std::ios::binary);
    AvlTree tree;
    REQUIRE_THROWS_AS(tree.deserializeBinary(iss), std::runtime_error);
}

TEST_CASE("AvlTree: serializeBinary выбрасывает, если поток в состоянии ошибки", "[AvlTree]")
{
    AvlTree tree;
    tree.insert("x");
    tree.insert("y");

    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios::badbit); 

    REQUIRE_THROWS_AS(tree.serializeBinary(oss), std::runtime_error);
}
