
#include "catch_amalgamated.hpp"
#include "forward_list.h"

#include <sstream>
#include <iostream>
#include <stdexcept>


// БАЗОВОЕ СОСТОЯНИЕ / ДЕСТРУКТОР / CLEAR


TEST_CASE("ForwardList: default ctor, пустой список, clear на пустом", "[ForwardList]")
{
    ForwardList list;

    // clear на пустом (через deserialize c пустой строкой)
    list.deserialize("");
    // ещё раз через пустой поток для deserializeText
    std::istringstream emptyStream;
    list.deserializeText(emptyStream);
}


// PUSH / POP (front/back)


TEST_CASE("ForwardList: pushFront / pushBack / popFront / popBack базовые случаи", "[ForwardList]")
{
    ForwardList list;

    SECTION("pushFront на пустой и непустой список")
    {
        list.pushFront("b");
        list.pushFront("a");
        // список: a -> b

        FNode* n1 = list.findNode("a");
        REQUIRE(n1 != nullptr);
        REQUIRE(n1->getValue() == "a");
        REQUIRE(n1->getNext() != nullptr);
        REQUIRE(n1->getNext()->getValue() == "b");
    }

    SECTION("pushBack на пустой и непустой список")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");
        // a -> b -> c

        FNode* a = list.findNode("a");
        FNode* b = list.findNode("b");
        FNode* c = list.findNode("c");

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(c != nullptr);
        REQUIRE(a->getNext() == b);
        REQUIRE(b->getNext() == c);
        REQUIRE(c->getNext() == nullptr);
    }

    SECTION("popFront на одном и нескольких элементах")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");

        list.popFront(); // убираем a
        REQUIRE(list.findNode("a") == nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);

        list.popFront(); // убираем b
        REQUIRE(list.findNode("b") == nullptr);
    }

    SECTION("popBack на одном и нескольких элементах")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");

        list.popBack(); // убираем c
        REQUIRE(list.findNode("c") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);

        list.popBack(); // убираем b
        REQUIRE(list.findNode("b") == nullptr);
    }
}


// removeByValue / findNode


TEST_CASE("ForwardList: removeByValue удаляет все вхождения", "[ForwardList]")
{
    ForwardList list;

    list.pushBack("x");
    list.pushBack("a");
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("a");
    list.pushBack("c");
    list.pushBack("a");
    // x -> a -> a -> b -> a -> c -> a

    SECTION("remove для существующего значения")
    {
        list.removeByValue("a");
        REQUIRE(list.findNode("a") == nullptr);
        REQUIRE(list.findNode("x") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }

    SECTION("remove для несуществующего значения не меняет список")
    {
        list.removeByValue("zzz");
        REQUIRE(list.findNode("x") != nullptr);
        REQUIRE(list.findNode("a") != nullptr);
    }
}

TEST_CASE("ForwardList: findNode возвращает nullptr, если не найдено", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("a");
    list.pushBack("b");

    REQUIRE(list.findNode("a") != nullptr);
    REQUIRE(list.findNode("b") != nullptr);
    REQUIRE(list.findNode("zzz") == nullptr);
}


// insertAfter / insertBefore / removeAfter / removeBefore


TEST_CASE("ForwardList: insertAfter вставляет после найденного значения", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("c");

    SECTION("insertAfter в середине")
    {
        list.insertAfter("b", "X"); // a -> b -> X -> c
        FNode* a = list.findNode("a");
        FNode* b = list.findNode("b");
        FNode* x = list.findNode("X");
        FNode* c = list.findNode("c");

        REQUIRE(a->getNext() == b);
        REQUIRE(b->getNext() == x);
        REQUIRE(x->getNext() == c);
    }

    SECTION("insertAfter в начале")
    {
        list.insertAfter("a", "X");
        FNode* a = list.findNode("a");
        FNode* x = list.findNode("X");
        REQUIRE(a->getNext() == x);
    }

    SECTION("insertAfter при отсутствии значения не меняет список")
    {
        list.insertAfter("zzz", "X");
        REQUIRE(list.findNode("X") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }
}

TEST_CASE("ForwardList: insertBefore корректно обрабатывает разные позиции", "[ForwardList]")
{
    ForwardList list;

    SECTION("insertBefore в пустом списке ничего не делает")
    {
        list.insertBefore("anything", "X");
        REQUIRE(list.findNode("X") == nullptr);
    }

    SECTION("insertBefore головы")
    {
        list.pushBack("b");
        list.pushBack("c");
        list.insertBefore("b", "a"); // a -> b -> c

        FNode* a = list.findNode("a");
        FNode* b = list.findNode("b");
        REQUIRE(a != nullptr);
        REQUIRE(a->getNext() == b);
    }

    SECTION("insertBefore в середине")
    {
        list.pushBack("a");
        list.pushBack("c");
        list.insertBefore("c", "b"); // a -> b -> c

        FNode* a = list.findNode("a");
        FNode* b = list.findNode("b");
        FNode* c = list.findNode("c");
        REQUIRE(a->getNext() == b);
        REQUIRE(b->getNext() == c);
    }

    SECTION("insertBefore при отсутствии значения не меняет список")
    {
        list.pushBack("a");
        list.insertBefore("zzz", "x");
        REQUIRE(list.findNode("x") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
    }
}

TEST_CASE("ForwardList: removeAfter корректно обрабатывает разные случаи", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("c");

    SECTION("removeAfter в середине")
    {
        list.removeAfter("a"); // удаляем b, остаётся a -> c
        REQUIRE(list.findNode("b") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }

    SECTION("removeAfter у последнего элемента не меняет список")
    {
        list.removeAfter("c"); // у c нет next
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }

    SECTION("removeAfter при отсутствии значения не меняет список")
    {
        list.removeAfter("zzz");
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }
}

TEST_CASE("ForwardList: removeBefore обрабатывает граничные случаи", "[ForwardList]")
{
    ForwardList list;

    SECTION("removeBefore при размере < 2 ничего не делает")
    {
        list.removeBefore("x"); // пустой
        list.pushBack("only");
        list.removeBefore("only"); // один элемент
        REQUIRE(list.findNode("only") != nullptr);
    }

    SECTION("removeBefore когда head->next == beforeValue -> popFront")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");
        // head = a, head->next = b, beforeValue = b

        list.removeBefore("b"); // удалит a (popFront)
        REQUIRE(list.findNode("a") == nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }

    SECTION("removeBefore в середине")
    {
        list.pushBack("x");
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");
        // x -> a -> b -> c
        // removeBefore("c") -> удаляем b

        list.removeBefore("c");
        REQUIRE(list.findNode("b") == nullptr);
        REQUIRE(list.findNode("x") != nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }

    SECTION("removeBefore при отсутствии значения не меняет список")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.removeBefore("zzz");
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
    }
}


// PRINT


TEST_CASE("ForwardList: print выводит список в ожидаемом формате", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("c");

    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    list.print();
    std::cout.rdbuf(oldBuf);

    REQUIRE(oss.str() == "[a->b->c]\n");
}


// ТЕКСТОВАЯ СЕРИАЛИЗАЦИЯ


TEST_CASE("ForwardList: serialize/deserialize text round-trip", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("one");
    list.pushBack("two");
    list.pushBack("three");

    std::string text = list.serialize();

    ForwardList restored;
    restored.deserialize(text);

    REQUIRE(restored.findNode("one") != nullptr);
    REQUIRE(restored.findNode("two") != nullptr);
    REQUIRE(restored.findNode("three") != nullptr);
    REQUIRE(restored.findNode("zzz") == nullptr);
}

TEST_CASE("ForwardList: deserializeText игнорирует пустые строки и очищает старые данные", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("old1");
    list.pushBack("old2");

    std::istringstream iss("line1\n\n\nline2\n\n");
    list.deserializeText(iss);

    REQUIRE(list.findNode("old1") == nullptr);
    REQUIRE(list.findNode("old2") == nullptr);
    REQUIRE(list.findNode("line1") != nullptr);
    REQUIRE(list.findNode("line2") != nullptr);
    REQUIRE(list.findNode("") == nullptr);
}

TEST_CASE("ForwardList: deserialize с пустой строкой делает список пустым", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("x");
    list.deserialize("");
    REQUIRE(list.findNode("x") == nullptr);
}


// БИНАРНАЯ СЕРИАЛИЗАЦИЯ: УСПЕШНЫЕ СЛУЧАИ


TEST_CASE("ForwardList: binary serialize/deserialize round-trip", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("alpha");
    list.pushBack("");
    list.pushBack("gamma");
    list.pushBack("русский текст");

    std::ostringstream oss(std::ios::binary);
    list.serializeBinary(oss);
    std::string bin = oss.str();

    std::istringstream iss(bin, std::ios::binary);
    ForwardList restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.findNode("alpha") != nullptr);
    REQUIRE(restored.findNode("") != nullptr);
    REQUIRE(restored.findNode("gamma") != nullptr);
    REQUIRE(restored.findNode("русский текст") != nullptr);
}

TEST_CASE("ForwardList: binary serialize/deserialize пустого списка", "[ForwardList]")
{
    ForwardList list;

    std::ostringstream oss(std::ios::binary);
    list.serializeBinary(oss);
    std::string bin = oss.str();
    REQUIRE_FALSE(bin.empty());

    std::istringstream iss(bin, std::ios::binary);
    ForwardList restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.findNode("anything") == nullptr);
}


// БИНАРНАЯ СЕРИАЛИЗАЦИЯ: ОШИБКИ


TEST_CASE("ForwardList: deserializeBinary бросает, если не может прочитать count", "[ForwardList]")
{
    std::string empty;
    std::istringstream iss(empty, std::ios::binary);
    ForwardList list;
    REQUIRE_THROWS_AS(list.deserializeBinary(iss), std::runtime_error);
}

TEST_CASE("ForwardList: serializeBinary бросает, если поток в состоянии ошибки", "[ForwardList]")
{
    ForwardList list;
    list.pushBack("x");
    list.pushBack("y");

    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios::badbit);

    REQUIRE_THROWS_AS(list.serializeBinary(oss), std::runtime_error);
}
