// benchmark.cpp
#include "catch_amalgamated.hpp"

#include "array.h"
#include "forward_list.h"
#include "list.h"
#include "stack.h"
#include "queue.h"
#include "hashtable.h"
#include "avltree.h"

#include <string>

// ===========================================================
// ---------------------- MYARRAY ----------------------------
// ===========================================================

TEST_CASE("Benchmark: MyArray pushBack", "[!benchmark]")
{
    MyArray arr;
    BENCHMARK("MyArray::pushBack (10000)") {
        for (int i = 0; i < 10000; ++i)
            arr.pushBack("value");
    };
}

TEST_CASE("Benchmark: MyArray insert at random", "[!benchmark]")
{
    MyArray arr;
    for (int i = 0; i < 5000; ++i)
        arr.pushBack("x");

    BENCHMARK("MyArray::insert middle (5000)") {
        arr.insert(arr.length / 2, "new");
        return arr.length;
    };
}


// ===========================================================
// ------------------- FORWARD_LIST --------------------------
// ===========================================================

TEST_CASE("Benchmark: ForwardList pushBack", "[!benchmark]")
{
    ForwardList list;
    BENCHMARK("ForwardList::pushBack (20000)") {
        for (int i = 0; i < 20000; ++i)
            list.pushBack("x");
    };
}

TEST_CASE("Benchmark: ForwardList find", "[!benchmark]")
{
    ForwardList list;
    for (int i = 0; i < 20000; ++i)
        list.pushBack("x" + std::to_string(i));

    BENCHMARK("ForwardList::findNode middle (20000)") {
        return list.findNode("x10000");
    };
}


// ===========================================================
// ------------------------ LIST -----------------------------
// ===========================================================

TEST_CASE("Benchmark: List pushBack", "[!benchmark]")
{
    List list;
    BENCHMARK("List::pushBack (20000)") {
        for (int i = 0; i < 20000; ++i)
            list.pushBack("x");
    };
}

TEST_CASE("Benchmark: List insertBefore", "[!benchmark]")
{
    List list;
    for (int i = 0; i < 20000; ++i)
        list.pushBack(std::to_string(i));

    BENCHMARK("List::insertBefore middle") {
        list.insertBefore("10000", "XX");
    };
}


// ===========================================================
// ------------------------ STACK -----------------------------
// ===========================================================

TEST_CASE("Benchmark: Stack push/pop", "[!benchmark]")
{
    Stack s;

    BENCHMARK("Stack::push 50000") {
        for (int i = 0; i < 50000; ++i)
            s.push("v");
    };

    for (int i = 0; i < 50000; ++i)
        s.pop();

    BENCHMARK("Stack::pop 50000") {
        for (int i = 0; i < 50000; ++i)
            s.pop();
    };
}


// ===========================================================
// ------------------------ QUEUE -----------------------------
// ===========================================================

TEST_CASE("Benchmark: Queue push/pop", "[!benchmark]")
{
    Queue q;

    BENCHMARK("Queue::push 50000") {
        for (int i = 0; i < 50000; ++i)
            q.push("v");
    };

    BENCHMARK("Queue::pop 50000") {
        for (int i = 0; i < 50000; ++i)
            q.pop();
    };
}


// ===========================================================
// ----------------------- HASHTABLE --------------------------
// ===========================================================

TEST_CASE("Benchmark: HashTable insert", "[!benchmark]")
{
    HashTable table;

    BENCHMARK("HashTable::insert 30000") {
        for (int i = 0; i < 30000; ++i)
            table.insert("key" + std::to_string(i), "v");
    };
}

TEST_CASE("Benchmark: HashTable find", "[!benchmark]")
{
    HashTable table;
    for (int i = 0; i < 30000; ++i)
        table.insert("key" + std::to_string(i), "v");

    BENCHMARK("HashTable::find (hit)") {
        return table.find("key15000");
    };
}


// ===========================================================
// ------------------- HASHTABLE OPEN -------------------------
// ===========================================================

TEST_CASE("Benchmark: HashTableOpen insert", "[!benchmark]")
{
    HashTableOpen table;

    BENCHMARK("HashTableOpen::insert 20000") {
        for (int i = 0; i < 20000; ++i)
            table.insert("k" + std::to_string(i), "v");
    };
}

TEST_CASE("Benchmark: HashTableOpen find", "[!benchmark]")
{
    HashTableOpen table;
    for (int i = 0; i < 20000; ++i)
        table.insert("k" + std::to_string(i), "v");

    BENCHMARK("HashTableOpen::find hit") {
        return table.find("k15000");
    };
}


// ===========================================================
// ----------------------- AVL TREE ---------------------------
// ===========================================================

TEST_CASE("Benchmark: AvlTree insert", "[!benchmark]")
{
    AvlTree t;
    BENCHMARK("AvlTree::insert 30000") {
        for (int i = 0; i < 30000; ++i)
            t.insert(std::to_string(i));
    };
}

TEST_CASE("Benchmark: AvlTree contains", "[!benchmark]")
{
    AvlTree t;
    for (int i = 0; i < 30000; ++i)
        t.insert(std::to_string(i));

    BENCHMARK("AvlTree::contains middle") {
        return t.contains("15000");
    };
}
//./tests_run "[!benchmark]"
