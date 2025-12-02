// test_array.cpp
#include "catch_amalgamated.hpp"
#include "array.h"

#include <sstream>
#include <stdexcept>
#include <string>

// =====================================================
// 1. Базовые свойства и конструкторы
// =====================================================

TEST_CASE("MyArray: конструктор по умолчанию и базовые свойства", "[MyArray]")
{
    MyArray arr; // по умолчанию

    REQUIRE(arr.size() == 0U);
    REQUIRE(arr.getCapacity() >= 1U);
    REQUIRE(arr.empty());
}

TEST_CASE("MyArray: пользовательская ёмкость и нормализация нуля", "[MyArray]")
{
    MyArray arr1(16);
    REQUIRE(arr1.getCapacity() == 16U);
    REQUIRE(arr1.size() == 0U);
    REQUIRE(arr1.empty());

    // При запросе capacity = 0 внутренняя ёмкость должна нормализоваться
    MyArray arr2(0);
    REQUIRE(arr2.getCapacity() == 1U);
    REQUIRE(arr2.size() == 0U);
    REQUIRE(arr2.empty());
}

// =====================================================
// 2. pushBack / оператор[] / at / set
// =====================================================

TEST_CASE("MyArray: pushBack добавляет элементы и при необходимости увеличивает ёмкость", "[MyArray]")
{
    MyArray arr(2);

    arr.pushBack("a");
    arr.pushBack("b");

    REQUIRE(arr.size() == 2U);
    REQUIRE(arr[0] == "a");
    REQUIRE(arr[1] == "b");

    // При добавлении третьего элемента должен сработать resize
    arr.pushBack("c");
    REQUIRE(arr.size() == 3U);
    REQUIRE(arr[2] == "c");
    REQUIRE(arr.getCapacity() >= 3U);

    // Проверяем at (с проверкой границ)
    REQUIRE(arr.at(0) == "a");
    REQUIRE(arr.at(1) == "b");
    REQUIRE(arr.at(2) == "c");

    const MyArray& constRef = arr;
    REQUIRE(constRef.at(0) == "a");
}

TEST_CASE("MyArray: at и set проверяют границы", "[MyArray]")
{
    MyArray arr;
    arr.pushBack("x");
    arr.pushBack("y");

    REQUIRE(arr.at(0) == "x");
    REQUIRE(arr.at(1) == "y");

    arr.set(0, "X");
    REQUIRE(arr.at(0) == "X");

    const MyArray& constArr = arr;
    REQUIRE(constArr.at(1) == "y");

    // Выход за границы должен приводить к исключению
    REQUIRE_THROWS_AS(arr.at(2), std::out_of_range);
    REQUIRE_THROWS_AS(constArr.at(2), std::out_of_range);
    REQUIRE_THROWS_AS(arr.set(2, "oops"), std::out_of_range);
}

TEST_CASE("MyArray: operator[] не проверяет границы, но даёт доступ к элементам", "[MyArray]")
{
    MyArray arr;
    arr.pushBack("hello");
    arr.pushBack("world");

    arr[0] = "HELLO";
    REQUIRE(arr[0] == "HELLO");

    const MyArray& constArr = arr;
    REQUIRE(constArr[1] == "world");
}

// =====================================================
// 3. insert / removeAt
// =====================================================

TEST_CASE("MyArray: insert вставляет в начало, середину и конец", "[MyArray]")
{
    MyArray arr(2);
    arr.pushBack("one");
    arr.pushBack("three");

    SECTION("вставка в начало")
    {
        arr.insert(0, "zero");
        REQUIRE(arr.size() == 3U);
        REQUIRE(arr[0] == "zero");
        REQUIRE(arr[1] == "one");
        REQUIRE(arr[2] == "three");
    }

    SECTION("вставка в середину")
    {
        arr.insert(1, "two");
        REQUIRE(arr.size() == 3U);
        REQUIRE(arr[0] == "one");
        REQUIRE(arr[1] == "two");
        REQUIRE(arr[2] == "three");
    }

    SECTION("вставка в конец (index == size)")
    {
        arr.insert(arr.size(), "four");
        REQUIRE(arr.size() == 3U);
        REQUIRE(arr[0] == "one");
        REQUIRE(arr[1] == "three");
        REQUIRE(arr[2] == "four");
    }
}

TEST_CASE("MyArray: insert выбрасывает исключение при индексе за пределами [0; size]", "[MyArray]")
{
    MyArray arr;
    arr.pushBack("a");
    arr.pushBack("b");

    REQUIRE_THROWS_AS(arr.insert(arr.size() + 1, "bad"), std::out_of_range);
}

TEST_CASE("MyArray: removeAt удаляет первый, средний и последний элементы", "[MyArray]")
{
    MyArray arr;
    arr.pushBack("a");
    arr.pushBack("b");
    arr.pushBack("c");
    arr.pushBack("d");

    SECTION("удаление первого элемента")
    {
        arr.removeAt(0);
        REQUIRE(arr.size() == 3U);
        REQUIRE(arr[0] == "b");
        REQUIRE(arr[1] == "c");
        REQUIRE(arr[2] == "d");
    }

    SECTION("удаление среднего элемента")
    {
        arr.removeAt(1);
        REQUIRE(arr.size() == 3U);
        REQUIRE(arr[0] == "a");
        REQUIRE(arr[1] == "c");
        REQUIRE(arr[2] == "d");
    }

    SECTION("удаление последнего элемента")
    {
        arr.removeAt(3);
        REQUIRE(arr.size() == 3U);
        REQUIRE(arr[0] == "a");
        REQUIRE(arr[1] == "b");
        REQUIRE(arr[2] == "c");
    }
}

TEST_CASE("MyArray: removeAt выбрасывает исключение при выходе за границы", "[MyArray]")
{
    MyArray arr;
    arr.pushBack("a");
    arr.pushBack("b");
    arr.pushBack("c");

    REQUIRE_THROWS_AS(arr.removeAt(3), std::out_of_range);
    REQUIRE_THROWS_AS(arr.removeAt(100), std::out_of_range);
}

// =====================================================
// 4. resize / empty
// =====================================================

TEST_CASE("MyArray: resize увеличивает и уменьшает ёмкость, при уменьшении обрезает лишние элементы", "[MyArray]")
{
    MyArray arr(2);
    arr.pushBack("one");
    arr.pushBack("two");

    SECTION("resize в большую ёмкость сохраняет элементы")
    {
        arr.resize(5);
        REQUIRE(arr.getCapacity() == 5U);
        REQUIRE(arr.size() == 2U);
        REQUIRE(arr[0] == "one");
        REQUIRE(arr[1] == "two");
    }

    SECTION("resize в меньшую ёмкость обрезает размер")
    {
        arr.pushBack("three");
        REQUIRE(arr.size() == 3U);

        arr.resize(2);
        REQUIRE(arr.getCapacity() == 2U);
        REQUIRE(arr.size() == 2U);
        REQUIRE(arr[0] == "one");
        REQUIRE(arr[1] == "two");
    }

    SECTION("resize в 0 нормализуется к capacity = 1", "[MyArray]")
    {
        arr.resize(0);
        REQUIRE(arr.getCapacity() == 1U);
        REQUIRE(arr.size() <= 1U);
    }
}

// =====================================================
// 5. Правило пяти: копирование, перемещение, swap
// =====================================================

TEST_CASE("MyArray: копирующий конструктор делает глубокую копию", "[MyArray]")
{
    MyArray original;
    original.pushBack("a");
    original.pushBack("b");
    original.pushBack("c");

    MyArray copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.getCapacity() == original.getCapacity());
    REQUIRE(copy.at(0) == "a");
    REQUIRE(copy.at(1) == "b");
    REQUIRE(copy.at(2) == "c");

    // Изменения оригинала не влияют на копию
    original.set(0, "changed");
    REQUIRE(copy.at(0) == "a");
}

TEST_CASE("MyArray: копирующее присваивание использует идиому copy-and-swap и корректно обрабатывает self-assignment", "[MyArray]")
{
    MyArray a;
    a.pushBack("x");
    a.pushBack("y");

    MyArray b;
    b.pushBack("1");
    b.pushBack("2");
    b.pushBack("3");

    b = a; // копирующее присваивание
    REQUIRE(b.size() == 2U);
    REQUIRE(b.at(0) == "x");
    REQUIRE(b.at(1) == "y");

    // Самоприсваивание не должно ломать объект
    b = b;
    REQUIRE(b.size() == 2U);
    REQUIRE(b.at(0) == "x");
    REQUIRE(b.at(1) == "y");
}

TEST_CASE("MyArray: перемещающий конструктор переносит владение данными", "[MyArray]")
{
    MyArray source;
    source.pushBack("foo");
    source.pushBack("bar");

    const std::size_t oldCap  = source.getCapacity();
    const std::size_t oldSize = source.size();

    MyArray moved(std::move(source));

    REQUIRE(moved.size() == oldSize);
    REQUIRE(moved.getCapacity() == oldCap);
    REQUIRE(moved.at(0) == "foo");
    REQUIRE(moved.at(1) == "bar");

    // Источник после перемещения должен быть "пустым" логически
    REQUIRE(source.size() == 0U);
}

TEST_CASE("MyArray: перемещающее присваивание переносит данные и очищает источник", "[MyArray]")
{
    MyArray source;
    source.pushBack("q");
    source.pushBack("w");
    const std::size_t oldCap  = source.getCapacity();
    const std::size_t oldSize = source.size();

    MyArray target;
    target.pushBack("old");

    target = std::move(source);

    REQUIRE(target.size() == oldSize);
    REQUIRE(target.getCapacity() == oldCap);
    REQUIRE(target.at(0) == "q");
    REQUIRE(target.at(1) == "w");

    REQUIRE(source.size() == 0U);
}

TEST_CASE("MyArray: swap обменивает содержимое двух массивов", "[MyArray]")
{
    MyArray a;
    a.pushBack("a1");
    a.pushBack("a2");

    MyArray b;
    b.pushBack("b1");

    const std::size_t aCap = a.getCapacity();
    const std::size_t bCap = b.getCapacity();

    a.swap(b);

    REQUIRE(a.size() == 1U);
    REQUIRE(a.at(0) == "b1");
    REQUIRE(a.getCapacity() == bCap);

    REQUIRE(b.size() == 2U);
    REQUIRE(b.at(0) == "a1");
    REQUIRE(b.at(1) == "a2");
    REQUIRE(b.getCapacity() == aCap);
}

// =====================================================
// 6. Текстовая сериализация
// =====================================================

TEST_CASE("MyArray: текстовая serialize/deserialize через потоки сохраняет порядок элементов", "[MyArray]")
{
    MyArray original;
    original.pushBack("line1");
    original.pushBack("line2");
    original.pushBack("");
    original.pushBack("last line");

    std::ostringstream oss;
    original.serializeText(oss);

    std::istringstream iss(oss.str());
    MyArray copy;
    copy.deserializeText(iss);

    REQUIRE(copy.size() == original.size());
    for (std::size_t i = 0; i < copy.size(); ++i) {
        REQUIRE(copy.at(i) == original.at(i));
    }
}

TEST_CASE("MyArray: текстовая serialize/deserialize через строковые методы", "[MyArray]")
{
    MyArray original;
    original.pushBack("hello");
    original.pushBack("world");

    const std::string data = original.serialize();

    MyArray restored;
    restored.deserialize(data);

    REQUIRE(restored.size() == 2U);
    REQUIRE(restored.at(0) == "hello");
    REQUIRE(restored.at(1) == "world");
}

// =====================================================
// 7. Бинарная сериализация
// =====================================================

TEST_CASE("MyArray: бинарная serialize/deserialize корректно восстанавливает массив", "[MyArray]")
{
    MyArray original;
    original.pushBack("alpha");
    original.pushBack("");
    original.pushBack("gamma with spaces");
    original.pushBack("русский текст");

    std::ostringstream oss(std::ios::binary);
    original.serializeBinary(oss);
    const std::string bin = oss.str();

    std::istringstream iss(bin, std::ios::binary);
    MyArray restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.size() == original.size());
    for (std::size_t i = 0; i < restored.size(); ++i) {
        REQUIRE(restored.at(i) == original.at(i));
    }
}
