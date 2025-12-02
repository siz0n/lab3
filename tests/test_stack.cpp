// test_stack.cpp
#include "catch_amalgamated.hpp"
#include "stack.h"

#include <sstream>
#include <string>

// =====================================================
// 1. Базовое состояние и очистка
// =====================================================

TEST_CASE("Stack: базовое состояние и очистка через deserialize", "[Stack]")
{
    Stack s;
    REQUIRE(s.empty());

    // Добавили элементы
    s.push("x");
    s.push("y");
    REQUIRE_FALSE(s.empty());

    // Очистка через текстовую десериализацию
    s.deserialize("");     // должен вызвать clear()
    REQUIRE(s.empty());
}

// =====================================================
// 2. push / pop / empty — базовая работа стека (LIFO)
// =====================================================

TEST_CASE("Stack: push/pop и корректный LIFO порядок", "[Stack]")
{
    Stack s;

    s.push("a");
    s.push("b");
    s.push("c");

    // Должно вытаскивать сверху вниз (LIFO)
    REQUIRE(s.pop() == "c");
    REQUIRE(s.pop() == "b");
    REQUIRE(s.pop() == "a");
    REQUIRE(s.empty());
}

// =====================================================
// 3. Текстовая сериализация и восстановление порядка
// =====================================================

TEST_CASE("Stack: serialize/deserialize (text) сохраняет порядок", "[Stack]")
{
    Stack s;
    s.push("bottom");
    s.push("middle");
    s.push("top");   // верхушка

    // Выгружаем в текст
    std::string text = s.serialize();

    // Загружаем обратно
    Stack restored;
    restored.deserialize(text);

    // Проверяем порядок
    REQUIRE(restored.pop() == "top");
    REQUIRE(restored.pop() == "middle");
    REQUIRE(restored.pop() == "bottom");
    REQUIRE(restored.empty());
}

// =====================================================
// 4. Текстовая десериализация пустой строки
// =====================================================

TEST_CASE("Stack: deserialize пустой строки делает стек пустым", "[Stack]")
{
    Stack s;
    s.push("x");
    s.push("y");

    s.deserialize("");   // clear() внутри

    REQUIRE(s.empty());
}

// =====================================================
// 5. Бинарная сериализация и восстановление порядка
// =====================================================

TEST_CASE("Stack: binary serialize/deserialize сохраняет порядок", "[Stack]")
{
    Stack s;
    s.push("bottom");
    s.push("");
    s.push("русский текст");
    s.push("top");

    // Сериализация в бинарный поток
    std::ostringstream oss(std::ios::binary);
    s.serializeBinary(oss);

    // Десериализация из бинарного
    std::istringstream iss(oss.str(), std::ios::binary);
    Stack restored;
    restored.deserializeBinary(iss);

    // Проверка корректного LIFO после восстановления
    REQUIRE(restored.pop() == "top");
    REQUIRE(restored.pop() == "русский текст");
    REQUIRE(restored.pop() == "");
    REQUIRE(restored.pop() == "bottom");
    REQUIRE(restored.empty());
}

// =====================================================
// 6. Бинарная сериализация пустого стека
// =====================================================

TEST_CASE("Stack: binary serialize/deserialize пустого стека", "[Stack]")
{
    Stack s;

    std::ostringstream oss(std::ios::binary);
    s.serializeBinary(oss);

    std::istringstream iss(oss.str(), std::ios::binary);
    Stack restored;
    restored.deserializeBinary(iss);

    REQUIRE(restored.empty());
}
