// test_queue.cpp
#include "catch_amalgamated.hpp"
#include "queue.h"

#include <sstream>
#include <iostream>
#include <stdexcept>

// -------------------------------------------------------------
// БАЗОВОЕ СОСТОЯНИЕ / CLEAR / ДЕСТРУКТОР
// -------------------------------------------------------------

TEST_CASE("Queue: конструктор по умолчанию, empty, clear", "[Queue]")
{
    Queue q;
    REQUIRE(q.size() == 0U);
    REQUIRE(q.empty());

    // clear вызывается через deserialize/deserializeText
    q.deserialize("");
    REQUIRE(q.size() == 0U);
    REQUIRE(q.empty());

    std::istringstream iss("");
    q.deserializeText(iss);
    REQUIRE(q.size() == 0U);
    REQUIRE(q.empty());
}

// -------------------------------------------------------------
// PUSH / POP / FRONT / BACK / SIZE / EMPTY
// -------------------------------------------------------------

TEST_CASE("Queue: push, pop, front, back — базовое поведение", "[Queue]")
{
    Queue q;

    SECTION("push в пустую и непустую очередь")
    {
        q.push("a");
        REQUIRE(q.size() == 1U);
        REQUIRE_FALSE(q.empty());
        REQUIRE(q.front() == "a");
        REQUIRE(q.back() == "a");

        q.push("b");
        q.push("c");
        REQUIRE(q.size() == 3U);
        REQUIRE(q.front() == "a");
        REQUIRE(q.back() == "c");
    }

    SECTION("pop из непустой очереди сохраняет FIFO-порядок")
    {
        q.push("one");
        q.push("two");
        q.push("three");

        REQUIRE(q.size() == 3U);
        REQUIRE(q.pop() == "one");
        REQUIRE(q.size() == 2U);
        REQUIRE(q.front() == "two");
        REQUIRE(q.back() == "three");

        REQUIRE(q.pop() == "two");
        REQUIRE(q.pop() == "three");
        REQUIRE(q.size() == 0U);
        REQUIRE(q.empty());
    }

    SECTION("pop из пустой очереди бросает исключение")
    {
        REQUIRE_THROWS_AS(q.pop(), std::out_of_range);
    }

    SECTION("front/back на пустой очереди бросают исключение")
    {
        REQUIRE_THROWS_AS(q.front(), std::out_of_range);
        REQUIRE_THROWS_AS(q.back(), std::out_of_range);
    }

    SECTION("после последнего pop front/back бросают исключение, очередь пустая")
    {
        q.push("x");
        REQUIRE(q.pop() == "x");
        REQUIRE(q.empty());
        REQUIRE(q.size() == 0U);
        REQUIRE_THROWS_AS(q.front(), std::out_of_range);
        REQUIRE_THROWS_AS(q.back(), std::out_of_range);
    }
}

// -------------------------------------------------------------
// PRINT
// -------------------------------------------------------------

TEST_CASE("Queue: print печатает ожидаемый формат", "[Queue]")
{
    Queue q;
    q.push("a");
    q.push("b");
    q.push("c");

    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    q.print();
    std::cout.rdbuf(oldBuf);

    REQUIRE(oss.str() == "[a, b, c]\n");
}

// -------------------------------------------------------------
// RULE OF FIVE: COPY/MOVE CTOR, COPY/MOVE ASSIGNMENT, SWAP
// -------------------------------------------------------------

TEST_CASE("Queue: конструктор копирования копирует элементы в порядке FIFO", "[Queue]")
{
    Queue original;
    original.push("one");
    original.push("two");
    original.push("three");

    Queue copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.front() == "one");
    REQUIRE(copy.back() == "three");

    // меняем оригинал — копия не должна измениться
    original.pop();      // удалили "one"
    original.push("four");

    REQUIRE(original.size() == 3U);
    REQUIRE(original.front() == "two");
    REQUIRE(original.back() == "four");

    REQUIRE(copy.size() == 3U);
    REQUIRE(copy.front() == "one");
    REQUIRE(copy.back() == "three");
}

TEST_CASE("Queue: оператор копирующего присваивания и self-assignment", "[Queue]")
{
    Queue a;
    a.push("x1");
    a.push("x2");

    Queue b;
    b.push("y1");
    b.push("y2");
    b.push("y3");

    // копирующее присваивание
    b = a;
    REQUIRE(b.size() == a.size());
    REQUIRE(b.front() == "x1");
    REQUIRE(b.back() == "x2");

    // самоприсваивание
    b = b;
    REQUIRE(b.size() == 2U);
    REQUIRE(b.front() == "x1");
    REQUIRE(b.back() == "x2");
}

TEST_CASE("Queue: конструктор перемещения переносит владение", "[Queue]")
{
    Queue src;
    src.push("a");
    src.push("b");
    src.push("c");

    std::size_t oldSize = src.size();

    Queue dst(std::move(src));

    REQUIRE(dst.size() == oldSize);
    REQUIRE(dst.front() == "a");
    REQUIRE(dst.back() == "c");

    REQUIRE(src.size() == 0U);
    REQUIRE(src.empty());
    REQUIRE_THROWS_AS(src.front(), std::out_of_range);
}

TEST_CASE("Queue: оператор перемещающего присваивания очищает приёмник и забирает данные", "[Queue]")
{
    Queue src;
    src.push("one");
    src.push("two");

    Queue dst;
    dst.push("old1");
    dst.push("old2");

    dst = std::move(src);

    REQUIRE(dst.size() == 2U);
    REQUIRE(dst.front() == "one");
    REQUIRE(dst.back() == "two");

    REQUIRE(src.size() == 0U);
    REQUIRE(src.empty());
    REQUIRE_THROWS_AS(src.front(), std::out_of_range);
}

TEST_CASE("Queue: swap обменивает содержимое очередей", "[Queue]")
{
    Queue a;
    a.push("a1");
    a.push("a2");

    Queue b;
    b.push("b1");

    std::size_t aSize = a.size();
    std::size_t bSize = b.size();

    a.swap(b);

    REQUIRE(a.size() == bSize);
    REQUIRE(a.front() == "b1");
    REQUIRE(a.back() == "b1");

    REQUIRE(b.size() == aSize);
    REQUIRE(b.front() == "a1");
    REQUIRE(b.back() == "a2");
}

// -------------------------------------------------------------
// ТЕКСТОВАЯ СЕРИАЛИЗАЦИЯ (serialize/deserialize)
// -------------------------------------------------------------

TEST_CASE("Queue: текстовая serialize/deserialize round-trip", "[Queue]")
{
    Queue q;
    q.push("first");
    q.push("second");
    q.push("third");

    std::string text = q.serialize();

    Queue restored;
    restored.deserialize(text);

    REQUIRE(restored.size() == 3U);
    REQUIRE(restored.pop() == "first");
    REQUIRE(restored.pop() == "second");
    REQUIRE(restored.pop() == "third");
    REQUIRE(restored.empty());
}

TEST_CASE("Queue: deserializeText очищает старые данные и игнорирует пустые строки", "[Queue]")
{
    Queue q;
    q.push("old1");
    q.push("old2");

    std::istringstream iss("one\n\n\n_two_\n\n");
    q.deserializeText(iss);

    REQUIRE(q.size() == 2U);
    REQUIRE(q.pop() == "one");
    REQUIRE(q.pop() == "_two_");
    REQUIRE(q.empty());
}

TEST_CASE("Queue: deserialize из пустой строки делает очередь пустой", "[Queue]")
{
    Queue q;
    q.push("x");
    q.deserialize("");

    REQUIRE(q.empty());
    REQUIRE(q.size() == 0U);
    REQUIRE_THROWS_AS(q.front(), std::out_of_range);
}

// -------------------------------------------------------------
// БИНАРНАЯ СЕРИАЛИЗАЦИЯ (УСПЕШНЫЕ СЛУЧАИ)
// -------------------------------------------------------------

TEST_CASE("Queue: бинарная serialize/deserialize round-trip", "[Queue]")
{
    Queue q;
    q.push("alpha");
    q.push("");
    q.push("gamma");
    q.push("русский текст");

    std::ostringstream oss(std::ios::binary);
    q.serializeBinary(oss);
    std::string bin = oss.str();

    std::istringstream iss(bin, std::ios::binary);
    Queue restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.size() == 4U);
    REQUIRE(restored.pop() == "alpha");
    REQUIRE(restored.pop() == "");
    REQUIRE(restored.pop() == "gamma");
    REQUIRE(restored.pop() == "русский текст");
    REQUIRE(restored.empty());
}

TEST_CASE("Queue: бинарная serialize/deserialize пустой очереди", "[Queue]")
{
    Queue q;

    std::ostringstream oss(std::ios::binary);
    q.serializeBinary(oss);
    std::string bin = oss.str();
    REQUIRE_FALSE(bin.empty()); // хотя бы счётчик/структура записались

    std::istringstream iss(bin, std::ios::binary);
    Queue restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.empty());
    REQUIRE(restored.size() == 0U);
}

// -------------------------------------------------------------
// БИНАРНАЯ СЕРИАЛИЗАЦИЯ — МИНИМАЛЬНЫЕ ОШИБКИ
// -------------------------------------------------------------

TEST_CASE("Queue: deserializeBinary бросает, если не может прочитать count", "[Queue]")
{
    std::string empty;
    std::istringstream iss(empty, std::ios::binary);
    Queue q;
    REQUIRE_THROWS_AS(q.deserializeBinary(iss), std::runtime_error);
}

TEST_CASE("Queue: serializeBinary бросает, если поток в состоянии ошибки", "[Queue]")
{
    Queue q;
    q.push("x");
    q.push("y");

    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios::badbit); // заранее ломаем поток

    REQUIRE_THROWS_AS(q.serializeBinary(oss), std::runtime_error);
}
