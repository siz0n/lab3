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


//  MYARRAY 


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
        arr.insert(arr.size() / 2, "new");
        return arr.size();
    };
}



//  FORWARD_LIST 


TEST_CASE("Benchmark: ForwardList pushBack", "[!benchmark]")
{
    ForwardList list;
    BENCHMARK("ForwardList::pushBack (2000)") {
        for (int i = 0; i < 2000; ++i)
            list.pushBack("x");
    };
}

TEST_CASE("Benchmark: ForwardList find", "[!benchmark]")
{
    ForwardList list;
    for (int i = 0; i < 2000; ++i)
        list.pushBack("x" + std::to_string(i));

    BENCHMARK("ForwardList::findNode middle (2000)") {
        return list.findNode("x1000");
    };
}



//  LIST 


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



//  STACK 


TEST_CASE("Benchmark: Stack operations", "[!benchmark][Stack]")
{
    BENCHMARK("Stack::push 5000")
    {
        Stack s;
        for (std::size_t i = 0; i < 5000; ++i) {
            s.push("value");
        }
        // стек живёт только внутри одной итерации бенчмарка
        return 0;
    };

    BENCHMARK("Stack::pop 5000")
    {
        Stack s;
        
        for (std::size_t i = 0; i < 5000; ++i) {
            s.push("value");
        }
        for (std::size_t i = 0; i < 5000; ++i) {
            s.pop();
        }
        return 0;
    };
}


//  QUEUE 


TEST_CASE("Benchmark: Queue operations", "[!benchmark][Queue]")
{
    BENCHMARK("Queue::push 5000")
    {
        Queue q;
        for (std::size_t i = 0; i < 5000; ++i) {
            q.push("value");
        }
        return 0;
    };

    BENCHMARK("Queue::pop 5000")
    {
        Queue q;
        // Заполняем очередь
        for (std::size_t i = 0; i < 5000; ++i) {
            q.push("value");
        }
        // А теперь 5000 pop — без выхода в пустую
        for (std::size_t i = 0; i < 5000; ++i) {
            q.pop();
        }
        return 0;
    };
}



//  HASHTABLE 


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



//  HASHTABLE OPEN 


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



//  AVL TREE 


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
//./tests_run "[!benchmark]" --benchmark-samples 10

