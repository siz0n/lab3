// test_list.cpp
#include "catch_amalgamated.hpp"
#include "list.h"

#include <sstream>
#include <iostream>
#include <stdexcept>


// БАЗОВОЕ СОСТОЯНИЕ / ДЕСТРУКТОР / CLEAR


TEST_CASE("List: конструктор по умолчанию и clear на пустом", "[List]")
{
    List list;

    // clear вызывается через deserialize/deserializeText
    list.deserialize("");
    std::istringstream iss;
    list.deserializeText(iss);
}


// PUSH / POP (front/back)


TEST_CASE("List: pushFront / pushBack / popFront / popBack", "[List]")
{
    List list;

    SECTION("pushFront на пустой и непустой список")
    {
        list.pushFront("b");
        list.pushFront("a");        // a <-> b

        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a->getPrev() == nullptr);
        REQUIRE(a->getNext() == b);
        REQUIRE(b->getPrev() == a);
        REQUIRE(b->getNext() == nullptr);
    }

    SECTION("pushBack на пустой и непустой список")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");         // a <-> b <-> c

        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");
        LNode* c = list.findNode("c");

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(c != nullptr);

        REQUIRE(a->getPrev() == nullptr);
        REQUIRE(a->getNext() == b);

        REQUIRE(b->getPrev() == a);
        REQUIRE(b->getNext() == c);

        REQUIRE(c->getPrev() == b);
        REQUIRE(c->getNext() == nullptr);
    }

    SECTION("popFront на пустом — no-op")
    {
        list.popFront();
    }

    SECTION("popFront на списке из одного элемента")
    {
        list.pushBack("x");
        list.popFront();
        REQUIRE(list.findNode("x") == nullptr);
    }

    SECTION("popFront на списке из нескольких элементов")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");
        list.popFront();            // удаляем a

        REQUIRE(list.findNode("a") == nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);

        LNode* b = list.findNode("b");
        LNode* c = list.findNode("c");
        REQUIRE(b->getPrev() == nullptr);
        REQUIRE(b->getNext() == c);
        REQUIRE(c->getPrev() == b);
    }

    SECTION("popBack на пустом — no-op")
    {
        list.popBack();
    }

    SECTION("popBack на списке из одного элемента")
    {
        list.pushBack("x");
        list.popBack();
        REQUIRE(list.findNode("x") == nullptr);
    }

    SECTION("popBack на списке из нескольких элементов")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");
        list.popBack();             // удаляем c

        REQUIRE(list.findNode("c") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);

        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");
        REQUIRE(a->getPrev() == nullptr);
        REQUIRE(a->getNext() == b);
        REQUIRE(b->getPrev() == a);
        REQUIRE(b->getNext() == nullptr);
    }
}


// removeByValue / findNode


TEST_CASE("List: removeByValue удаляет все вхождения и правит head/tail", "[List]")
{
    List list;
    list.pushBack("x");
    list.pushBack("a");
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("a");
    list.pushBack("c");
    list.pushBack("a");
    // x <-> a <-> a <-> b <-> a <-> c <-> a

    SECTION("удаление значения, встречающегося несколько раз")
    {
        list.removeByValue("a");
        REQUIRE(list.findNode("a") == nullptr);
        REQUIRE(list.findNode("x") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);

        LNode* x = list.findNode("x");
        LNode* b = list.findNode("b");
        LNode* c = list.findNode("c");

        REQUIRE(x->getPrev() == nullptr);
        REQUIRE(x->getNext() == b);
        REQUIRE(b->getPrev() == x);
        REQUIRE(b->getNext() == c);
        REQUIRE(c->getPrev() == b);
        REQUIRE(c->getNext() == nullptr);
    }

    SECTION("удаление несуществующего значения не меняет список")
    {
        list.removeByValue("zzz");
        REQUIRE(list.findNode("x") != nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }
}

TEST_CASE("List: findNode возвращает nullptr, если не найдено", "[List]")
{
    List list;
    list.pushBack("a");
    list.pushBack("b");

    REQUIRE(list.findNode("a") != nullptr);
    REQUIRE(list.findNode("b") != nullptr);
    REQUIRE(list.findNode("zzz") == nullptr);
}


// insertAfter / insertBefore / removeAfter / removeBefore


TEST_CASE("List: insertAfter в середину/хвост и no-op если не найдено", "[List]")
{
    List list;
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("c");

    SECTION("insertAfter в середину")
    {
        list.insertAfter("b", "X");     // a <-> b <-> X <-> c
        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");
        LNode* x = list.findNode("X");
        LNode* c = list.findNode("c");

        REQUIRE(a->getNext() == b);
        REQUIRE(b->getPrev() == a);
        REQUIRE(b->getNext() == x);
        REQUIRE(x->getPrev() == b);
        REQUIRE(x->getNext() == c);
        REQUIRE(c->getPrev() == x);
    }

    SECTION("insertAfter в хвост обновляет tail", "[List]")
    {
        list.insertAfter("c", "X");     // a <-> b <-> c <-> X
        LNode* c = list.findNode("c");
        LNode* x = list.findNode("X");

        REQUIRE(c->getNext() == x);
        REQUIRE(x->getPrev() == c);
        REQUIRE(x->getNext() == nullptr);
    }

    SECTION("insertAfter несуществующего значения не меняет список", "[List]")
    {
        list.insertAfter("zzz", "X");
        REQUIRE(list.findNode("X") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }
}

TEST_CASE("List: insertBefore для головы/середины и no-op если не найдено", "[List]")
{
    List list;

    SECTION("insertBefore в пустой список — ничего не делает")
    {
        list.insertBefore("anything", "X");
        REQUIRE(list.findNode("X") == nullptr);
    }

    SECTION("insertBefore перед головой", "[List]")
    {
        list.pushBack("b");
        list.pushBack("c");
        list.insertBefore("b", "a");    // a <-> b <-> c

        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a->getPrev() == nullptr);
        REQUIRE(a->getNext() == b);
        REQUIRE(b->getPrev() == a);
    }

    SECTION("insertBefore в середину", "[List]")
    {
        list.pushBack("a");
        list.pushBack("c");
        list.insertBefore("c", "b");    // a <-> b <-> c

        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");
        LNode* c = list.findNode("c");

        REQUIRE(a->getNext() == b);
        REQUIRE(b->getPrev() == a);
        REQUIRE(b->getNext() == c);
        REQUIRE(c->getPrev() == b);
    }

    SECTION("insertBefore при отсутствии значения не меняет список", "[List]")
    {
        list.pushBack("a");
        list.insertBefore("zzz", "x");
        REQUIRE(list.findNode("x") == nullptr);
        REQUIRE(list.findNode("a") != nullptr);
    }
}

TEST_CASE("List: removeAfter обрабатывает середину/хвост и not-found", "[List]")
{
    List list;
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("c");

    SECTION("removeAfter в середине", "[List]")
    {
        list.removeAfter("a");          // удаляем b, остаётся a <-> c
        REQUIRE(list.findNode("b") == nullptr);
        LNode* a = list.findNode("a");
        LNode* c = list.findNode("c");
        REQUIRE(a->getNext() == c);
        REQUIRE(c->getPrev() == a);
    }

    SECTION("removeAfter у последнего элемента ничего не делает", "[List]")
    {
        list.removeAfter("c");          // у c нет next
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }

    SECTION("removeAfter несуществующего значения ничего не делает", "[List]")
    {
        list.removeAfter("zzz");
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);
    }
}

TEST_CASE("List: removeBefore обрабатывает голову/середину и крайние случаи", "[List]")
{
    List list;

    SECTION("removeBefore при единственном элементе — ничего", "[List]")
    {
        list.pushBack("only");
        list.removeBefore("only");
        REQUIRE(list.findNode("only") != nullptr);
    }

    SECTION("removeBefore, когда beforeValue = head и prev нет — ничего", "[List]")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.removeBefore("a");
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
    }

    SECTION("removeBefore в середине удаляет head при необходимости", "[List]")
    {
        list.pushBack("x");
        list.pushBack("a");
        list.pushBack("b");
        // x <-> a <-> b
        // removeBefore("a") -> удаляем x, голова теперь a

        list.removeBefore("a");
        REQUIRE(list.findNode("x") == nullptr);
        LNode* a = list.findNode("a");
        LNode* b = list.findNode("b");
        REQUIRE(a->getPrev() == nullptr);
        REQUIRE(a->getNext() == b);
        REQUIRE(b->getPrev() == a);
    }

    SECTION("removeBefore в общем случае (середина)", "[List]")
    {
        list.pushBack("x");
        list.pushBack("a");
        list.pushBack("b");
        list.pushBack("c");
        // x <-> a <-> b <-> c
        // removeBefore("c") -> удаляем b

        list.removeBefore("c");
        REQUIRE(list.findNode("b") == nullptr);
        REQUIRE(list.findNode("x") != nullptr);
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("c") != nullptr);

        LNode* a = list.findNode("a");
        LNode* c = list.findNode("c");
        REQUIRE(a->getNext() == c);
        REQUIRE(c->getPrev() == a);
    }

    SECTION("removeBefore при отсутствии значения не меняет список", "[List]")
    {
        list.pushBack("a");
        list.pushBack("b");
        list.removeBefore("zzz");
        REQUIRE(list.findNode("a") != nullptr);
        REQUIRE(list.findNode("b") != nullptr);
    }
}


// PRINT


TEST_CASE("List: print печатает список в ожидаемом формате", "[List]")
{
    List list;
    list.pushBack("a");
    list.pushBack("b");
    list.pushBack("c");

    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());
    list.print();
    std::cout.rdbuf(oldBuf);

    REQUIRE(oss.str() == "[a<->b<->c]\n");
}


// ТЕКСТОВАЯ СЕРИАЛИЗАЦИЯ


TEST_CASE("List: serialize/deserialize text round-trip", "[List]")
{
    List list;
    list.pushBack("one");
    list.pushBack("two");
    list.pushBack("three");

    std::string text = list.serialize();

    List restored;
    restored.deserialize(text);

    REQUIRE(restored.findNode("one") != nullptr);
    REQUIRE(restored.findNode("two") != nullptr);
    REQUIRE(restored.findNode("three") != nullptr);
    REQUIRE(restored.findNode("zzz") == nullptr);
}

TEST_CASE("List: deserializeText очищает старые данные и игнорирует пустые строки", "[List]")
{
    List list;
    list.pushBack("old1");
    list.pushBack("old2");

    std::istringstream iss("line1\n\nline2\n\n");
    list.deserializeText(iss);

    REQUIRE(list.findNode("old1") == nullptr);
    REQUIRE(list.findNode("old2") == nullptr);
    REQUIRE(list.findNode("line1") != nullptr);
    REQUIRE(list.findNode("line2") != nullptr);
    REQUIRE(list.findNode("") == nullptr);
}

TEST_CASE("List: deserialize из пустой строки делает список пустым", "[List]")
{
    List list;
    list.pushBack("x");
    list.deserialize("");
    REQUIRE(list.findNode("x") == nullptr);
}


// БИНАРНАЯ СЕРИАЛИЗАЦИЯ: УСПЕШНЫЕ КЕЙСЫ


TEST_CASE("List: binary serialize/deserialize round-trip", "[List]")
{
    List list;
    list.pushBack("alpha");
    list.pushBack("");
    list.pushBack("gamma");
    list.pushBack("русский текст");

    std::ostringstream oss(std::ios::binary);
    list.serializeBinary(oss);
    std::string bin = oss.str();

    std::istringstream iss(bin, std::ios::binary);
    List restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.findNode("alpha") != nullptr);
    REQUIRE(restored.findNode("") != nullptr);
    REQUIRE(restored.findNode("gamma") != nullptr);
    REQUIRE(restored.findNode("русский текст") != nullptr);
}

TEST_CASE("List: binary serialize/deserialize пустого списка", "[List]")
{
    List list;

    std::ostringstream oss(std::ios::binary);
    list.serializeBinary(oss);
    std::string bin = oss.str();
    REQUIRE_FALSE(bin.empty()); // хотя бы count записался

    std::istringstream iss(bin, std::ios::binary);
    List restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.findNode("anything") == nullptr);
}


// БИНАРНАЯ СЕРИАЛИЗАЦИЯ: МИНИМАЛЬНЫЕ ОШИБКИ


TEST_CASE("List: deserializeBinary бросает, если не может прочитать count", "[List]")
{
    std::string empty;
    std::istringstream iss(empty, std::ios::binary);
    List list;
    REQUIRE_THROWS_AS(list.deserializeBinary(iss), std::runtime_error);
}

TEST_CASE("List: serializeBinary бросает, если поток в состоянии ошибки", "[List]")
{
    List list;
    list.pushBack("x");
    list.pushBack("y");

    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios::badbit);    // заранее ломаем поток

    REQUIRE_THROWS_AS(list.serializeBinary(oss), std::runtime_error);
}
