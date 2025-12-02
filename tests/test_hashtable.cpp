// test_hashtable.cpp
#include "catch_amalgamated.hpp"
#include "hashtable.h"

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string>


// HashTable — базовые свойства / конструкторы


TEST_CASE("HashTable: конструктор по умолчанию и базовые свойства", "[HashTable]")
{
    HashTable table;
    REQUIRE(table.size() == 0U);
    REQUIRE(table.empty());
    REQUIRE(table.bucketCount() >= 1U);

    // clear на пустой
    table.clear();
    REQUIRE(table.size() == 0U);

    // print (для покрытия)
    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    table.print();
    std::cout.rdbuf(oldBuf);
}

TEST_CASE("HashTable: пользовательское число бакетов и нормализация нуля", "[HashTable]")
{
    HashTable t1(16);
    REQUIRE(t1.bucketCount() == 16U);

    HashTable t2(0);
    REQUIRE(t2.bucketCount() == 1U);
}


// HashTable — insert / erase / find / operator[]


TEST_CASE("HashTable: вставка, поиск, удаление, обновление", "[HashTable]")
{
    HashTable table;
    REQUIRE(table.empty());

    table.insert("a", "1");
    table.insert("b", "2");
    table.insert("c", "3");

    REQUIRE(table.size() == 3U);
    REQUIRE_FALSE(table.empty());

    // find (неконстантный)
    std::string* pa = table.find("a");
    REQUIRE(pa != nullptr);
    REQUIRE(*pa == "1");

    // find (константный)
    const HashTable& cref = table;
    const std::string* pb = cref.find("b");
    REQUIRE(pb != nullptr);
    REQUIRE(*pb == "2");

    REQUIRE(table.find("zzz") == nullptr);
    REQUIRE(cref.find("zzz") == nullptr);

    // обновление существующего ключа
    table.insert("b", "UPDATED");
    REQUIRE(table.size() == 3U);
    pb = cref.find("b");
    REQUIRE(pb != nullptr);
    REQUIRE(*pb == "UPDATED");

    // erase существующего
    table.erase("b");
    REQUIRE(table.size() == 2U);
    REQUIRE(table.find("b") == nullptr);

    // erase несуществующего — без изменений
    table.erase("nope");
    REQUIRE(table.size() == 2U);
}

TEST_CASE("HashTable: operator[] вставляет при промахе и возвращает ссылку", "[HashTable]")
{
    HashTable table;
    table.insert("x", "42");

    // существующий
    std::string& ref = table["x"];
    REQUIRE(ref == "42");
    ref = "99";
    REQUIRE(*table.find("x") == "99");

    // отсутствующий — вставляется с пустой строкой
    std::string& refNew = table["newKey"];
    REQUIRE(refNew.empty());
    REQUIRE(table.size() == 2U);
    refNew = "val";
    REQUIRE(*table.find("newKey") == "val");
}

TEST_CASE("HashTable: rehash вызывается при росте нагрузки", "[HashTable]")
{
    HashTable table(2); // маленькое число бакетов для быстрого rehash

    std::size_t oldBuckets = table.bucketCount();
    REQUIRE(oldBuckets == 2U);

    table.insert("k1", "v1");
    table.insert("k2", "v2");
    table.insert("k3", "v3"); // при вставке должен сработать rehash

    REQUIRE(table.size() == 3U);
    REQUIRE(table.bucketCount() >= oldBuckets);

    REQUIRE(*table.find("k1") == "v1");
    REQUIRE(*table.find("k2") == "v2");
    REQUIRE(*table.find("k3") == "v3");
}

TEST_CASE("HashTable: clear удаляет элементы, но сохраняет количество бакетов", "[HashTable]")
{
    HashTable table;
    table.insert("a", "1");
    table.insert("b", "2");

    std::size_t buckets = table.bucketCount();
    table.clear();

    REQUIRE(table.size() == 0U);
    REQUIRE(table.empty());
    REQUIRE(table.bucketCount() == buckets);
    REQUIRE(table.find("a") == nullptr);
    REQUIRE(table.find("b") == nullptr);
}


// HashTable — Rule of Five: copy/move


TEST_CASE("HashTable: конструктор копирования делает глубокую копию", "[HashTable]")
{
    HashTable original;
    original.insert("a", "1");
    original.insert("b", "2");
    original.insert("c", "3");

    HashTable copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(*copy.find("a") == "1");
    REQUIRE(*copy.find("b") == "2");
    REQUIRE(*copy.find("c") == "3");

    // изменения оригинала не влияют на копию
    original.insert("d", "4");
    REQUIRE(original.size() == 4U);
    REQUIRE(copy.size() == 3U);
    REQUIRE(copy.find("d") == nullptr);
}

TEST_CASE("HashTable: оператор копирующего присваивания и самоприсваивание", "[HashTable]")
{
    HashTable a;
    a.insert("x", "1");
    a.insert("y", "2");

    HashTable b;
    b.insert("old", "q");

    b = a;
    REQUIRE(b.size() == a.size());
    REQUIRE(*b.find("x") == "1");
    REQUIRE(*b.find("y") == "2");
    REQUIRE(b.find("old") == nullptr);

    // self-assignment
    b = b;
    REQUIRE(b.size() == 2U);
    REQUIRE(*b.find("x") == "1");
    REQUIRE(*b.find("y") == "2");
}

TEST_CASE("HashTable: move-конструктор переносит бакеты", "[HashTable]")
{
    HashTable src;
    src.insert("a", "1");
    src.insert("b", "2");

    std::size_t oldBuckets = src.bucketCount();
    std::size_t oldSize    = src.size();

    HashTable dst(std::move(src));

    REQUIRE(dst.size() == oldSize);
    REQUIRE(dst.bucketCount() == oldBuckets);
    REQUIRE(*dst.find("a") == "1");
    REQUIRE(*dst.find("b") == "2");

    REQUIRE(src.size() == 0U);
    REQUIRE(src.bucketCount() == 0U);
    REQUIRE(src.find("a") == nullptr);
}

TEST_CASE("HashTable: move-присваивание переносит бакеты и очищает источник", "[HashTable]")
{
    HashTable src;
    src.insert("k1", "v1");
    src.insert("k2", "v2");

    HashTable dst;
    dst.insert("old", "q");

    dst = std::move(src);

    REQUIRE(dst.size() == 2U);
    REQUIRE(dst.find("old") == nullptr);
    REQUIRE(*dst.find("k1") == "v1");
    REQUIRE(*dst.find("k2") == "v2");

    REQUIRE(src.size() == 0U);
    REQUIRE(src.bucketCount() == 0U);
}


// HashTable — текстовая сериализация


TEST_CASE("HashTable: текстовая serialize/deserialize round-trip", "[HashTable]")
{
    HashTable table;
    table.insert("alpha", "1");
    table.insert("beta", "2");
    table.insert("gamma", "3");

    std::string text = table.serialize();

    HashTable restored;
    restored.deserialize(text);

    REQUIRE(restored.size() == 3U);
    REQUIRE(*restored.find("alpha") == "1");
    REQUIRE(*restored.find("beta") == "2");
    REQUIRE(*restored.find("gamma") == "3");
}

TEST_CASE("HashTable: deserializeText с нечисловым count оставляет таблицу пустой", "[HashTable]")
{
    HashTable table;
    table.insert("x", "1");

    std::istringstream iss("not_a_number\nkey\tvalue\n");
    table.deserializeText(iss);

    REQUIRE(table.size() == 0U);
    REQUIRE(table.empty());
}

TEST_CASE("HashTable: deserializeText бросает, если не удалось прочитать key/value", "[HashTable]")
{
    HashTable table;

    SECTION("нет строк после count")
    {
        std::istringstream iss("1\n");
        REQUIRE_THROWS_AS(table.deserializeText(iss), std::runtime_error);
    }

    SECTION("есть строка с ключом, но без значения")
    {
        std::istringstream iss("1\nkey_without_value_line\n");
        REQUIRE_THROWS_AS(table.deserializeText(iss), std::runtime_error);
    }
}


// HashTable — бинарная сериализация (успех / минимальные ошибки)


TEST_CASE("HashTable: binary serialize/deserialize round-trip", "[HashTable]")
{
    HashTable table;
    table.insert("k1", "v1");
    table.insert("k2", "");
    table.insert("русский", "текст");

    std::ostringstream oss(std::ios::binary);
    table.serializeBinary(oss);
    std::string data = oss.str();

    std::istringstream iss(data, std::ios::binary);
    HashTable restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.size() == table.size());
    REQUIRE(*restored.find("k1") == "v1");
    REQUIRE(*restored.find("k2") == "");
    REQUIRE(*restored.find("русский") == "текст");
}

TEST_CASE("HashTable: deserializeBinary бросает, если не может прочитать count", "[HashTable]")
{
    std::string empty;
    std::istringstream iss(empty, std::ios::binary);
    HashTable table;
    REQUIRE_THROWS_AS(table.deserializeBinary(iss), std::runtime_error);
}

TEST_CASE("HashTable: serializeBinary бросает, если поток в состоянии ошибки", "[HashTable]")
{
    HashTable table;
    table.insert("x", "1");

    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios::badbit);

    REQUIRE_THROWS_AS(table.serializeBinary(oss), std::runtime_error);
}

// =============================================================
// HashTableOpen — тесты
// =============================================================

TEST_CASE("HashTableOpen: конструктор по умолчанию и свойства", "[HashTableOpen]")
{
    HashTableOpen table;
    REQUIRE(table.size() == 0U);
    REQUIRE(table.empty());
    REQUIRE(table.capacity() >= 1U);

    table.clear();
    REQUIRE(table.size() == 0U);

    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    table.print();
    std::cout.rdbuf(oldBuf);
}

TEST_CASE("HashTableOpen: пользовательская ёмкость и нормализация нуля", "[HashTableOpen]")
{
    HashTableOpen t1(16);
    REQUIRE(t1.capacity() == 16U);

    HashTableOpen t2(0);
    REQUIRE(t2.capacity() == 1U);
}

TEST_CASE("HashTableOpen: insert, find, erase, update", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("a", "1");
    table.insert("b", "2");
    table.insert("c", "3");

    REQUIRE(table.size() == 3U);
    REQUIRE_FALSE(table.empty());

    std::string* pa = table.find("a");
    REQUIRE(pa != nullptr);
    REQUIRE(*pa == "1");

    const HashTableOpen& cref = table;
    const std::string* pb = cref.find("b");
    REQUIRE(pb != nullptr);
    REQUIRE(*pb == "2");

    REQUIRE(table.find("zzz") == nullptr);
    REQUIRE(cref.find("zzz") == nullptr);

    table.insert("b", "UPDATED");
    REQUIRE(table.size() == 3U);
    REQUIRE(*table.find("b") == "UPDATED");

    table.erase("b");
    REQUIRE(table.size() == 2U);
    REQUIRE(table.find("b") == nullptr);

    table.erase("nope");
    REQUIRE(table.size() == 2U);
}

TEST_CASE("HashTableOpen: operator[] вставляет по умолчанию и возвращает ссылку", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("x", "42");

    std::string& ref = table["x"];
    REQUIRE(ref == "42");
    ref = "99";
    REQUIRE(*table.find("x") == "99");

    std::string& refNew = table["newKey"];
    REQUIRE(refNew.empty());
    REQUIRE(table.size() == 2U);
    refNew = "val";
    REQUIRE(*table.find("newKey") == "val");
}

TEST_CASE("HashTableOpen: rehash увеличивает capacity", "[HashTableOpen]")
{
    HashTableOpen table(2);
    std::size_t oldCap = table.capacity();
    REQUIRE(oldCap == 2U);

    table.insert("k1", "v1");
    table.insert("k2", "v2"); // при вставке должен сработать rehash

    REQUIRE(table.capacity() >= oldCap);
    REQUIRE(table.size() == 2U);
    REQUIRE(*table.find("k1") == "v1");
    REQUIRE(*table.find("k2") == "v2");
}

TEST_CASE("HashTableOpen: clear очищает элементы, но оставляет ёмкость", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("a", "1");
    table.insert("b", "2");

    std::size_t cap = table.capacity();
    table.clear();

    REQUIRE(table.size() == 0U);
    REQUIRE(table.empty());
    REQUIRE(table.capacity() == cap);
    REQUIRE(table.find("a") == nullptr);
    REQUIRE(table.find("b") == nullptr);
}


// HashTableOpen — Rule of Five


TEST_CASE("HashTableOpen: конструктор копирования копирует таблицу", "[HashTableOpen]")
{
    HashTableOpen original;
    original.insert("a", "1");
    original.insert("b", "2");

    HashTableOpen copy(original);
    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.capacity() == original.capacity());
    REQUIRE(*copy.find("a") == "1");
    REQUIRE(*copy.find("b") == "2");

    original.insert("c", "3");
    REQUIRE(original.size() == 3U);
    REQUIRE(copy.size() == 2U);
}

TEST_CASE("HashTableOpen: копирующее присваивание и самоприсваивание", "[HashTableOpen]")
{
    HashTableOpen a;
    a.insert("x", "1");
    a.insert("y", "2");

    HashTableOpen b;
    b.insert("old", "q");

    b = a;
    REQUIRE(b.size() == a.size());
    REQUIRE(*b.find("x") == "1");
    REQUIRE(*b.find("y") == "2");
    REQUIRE(b.find("old") == nullptr);

    b = b;
    REQUIRE(b.size() == 2U);
    REQUIRE(*b.find("x") == "1");
    REQUIRE(*b.find("y") == "2");
}

TEST_CASE("HashTableOpen: move-конструктор переносит таблицу", "[HashTableOpen]")
{
    HashTableOpen src;
    src.insert("a", "1");
    src.insert("b", "2");

    std::size_t oldCap = src.capacity();
    std::size_t oldSize = src.size();

    HashTableOpen dst(std::move(src));

    REQUIRE(dst.size() == oldSize);
    REQUIRE(dst.capacity() == oldCap);
    REQUIRE(*dst.find("a") == "1");
    REQUIRE(*dst.find("b") == "2");

    REQUIRE(src.size() == 0U);
    REQUIRE(src.capacity() == 0U);
}

TEST_CASE("HashTableOpen: move-присваивание переносит и очищает источник", "[HashTableOpen]")
{
    HashTableOpen src;
    src.insert("k1", "v1");
    src.insert("k2", "v2");

    HashTableOpen dst;
    dst.insert("old", "q");

    dst = std::move(src);

    REQUIRE(dst.size() == 2U);
    REQUIRE(dst.find("old") == nullptr);
    REQUIRE(*dst.find("k1") == "v1");
    REQUIRE(*dst.find("k2") == "v2");

    REQUIRE(src.size() == 0U);
    REQUIRE(src.capacity() == 0U);
}


// HashTableOpen — текстовая сериализация


TEST_CASE("HashTableOpen: текстовая serialize/deserialize round-trip", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("alpha", "1");
    table.insert("beta", "2");
    table.insert("gamma", "3");

    std::string text = table.serialize();

    HashTableOpen restored;
    restored.deserialize(text);

    REQUIRE(restored.size() == 3U);
    REQUIRE(*restored.find("alpha") == "1");
    REQUIRE(*restored.find("beta") == "2");
    REQUIRE(*restored.find("gamma") == "3");
}

TEST_CASE("HashTableOpen: deserializeText с нечисловым count очищает таблицу", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("x", "1");

    std::istringstream iss("not_number\nkey\tvalue\n");
    table.deserializeText(iss);

    REQUIRE(table.size() == 0U);
    REQUIRE(table.empty());
}

TEST_CASE("HashTableOpen: deserializeText бросает при битых строках key/value", "[HashTableOpen]")
{
    HashTableOpen table;

    SECTION("нет строк после count")
    {
        std::istringstream iss("1\n");
        REQUIRE_THROWS_AS(table.deserializeText(iss), std::runtime_error);
    }

    SECTION("есть строка только с ключом")
    {
        std::istringstream iss("1\nkey_only_line\n");
        REQUIRE_THROWS_AS(table.deserializeText(iss), std::runtime_error);
    }
}


// HashTableOpen — бинарная сериализация (успех / минимум ошибок)


TEST_CASE("HashTableOpen: binary serialize/deserialize ", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("k1", "v1");
    table.insert("k2", "");
    table.insert("русский", "текст");

    std::ostringstream oss(std::ios::binary);
    table.serializeBinary(oss);
    std::string data = oss.str();

    std::istringstream iss(data, std::ios::binary);
    HashTableOpen restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.size() == table.size());
    REQUIRE(*restored.find("k1") == "v1");
    REQUIRE(*restored.find("k2") == "");
    REQUIRE(*restored.find("русский") == "текст");
}

TEST_CASE("HashTableOpen: binary serialize/deserialize с удалёнными элементами", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("a", "1");
    table.insert("b", "2");
    table.erase("a"); // помечаем как DELETED

    std::ostringstream oss(std::ios::binary);
    table.serializeBinary(oss);
    std::string data = oss.str();

    std::istringstream iss(data, std::ios::binary);
    HashTableOpen restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.find("a") == nullptr);
    REQUIRE(*restored.find("b") == "2");
}

TEST_CASE("HashTableOpen: deserializeBinary бросает, если не может прочитать count", "[HashTableOpen]")
{
    std::string empty;
    std::istringstream iss(empty, std::ios::binary);
    HashTableOpen table;
    REQUIRE_THROWS_AS(table.deserializeBinary(iss), std::runtime_error);
}

TEST_CASE("HashTableOpen: serializeBinary бросает, если поток в состоянии ошибки", "[HashTableOpen]")
{
    HashTableOpen table;
    table.insert("x", "1");

    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios::badbit);

    REQUIRE_THROWS_AS(table.serializeBinary(oss), std::runtime_error);
}
