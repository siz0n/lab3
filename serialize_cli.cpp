// serialize_cli.cpp
#include "cont/array.h"
#include "cont/forward_list.h"
#include "cont/list.h"
#include "cont/stack.h"
#include "cont/queue.h"
#include "cont/hashtable.h"
#include "cont/avltree.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdint>

// =======================
// Типы структур данных
// =======================

enum class DSKind
{
    ARRAY,
    FLIST,
    LLIST,
    STACK,
    QUEUE,
    AVL,
    HCHAIN, // цепная хеш-таблица
    HOPEN   // хеш-таблица с открытой адресацией
};

struct DSRecord
{
    std::string name;
    DSKind      kind;
    void*       ptr;
};

static constexpr int MAX_DS = 64;

// =======================
// Класс DBMS
// =======================

class DBMS
{
public:
    DBMS();
    ~DBMS();

    void clear();

    // текстовый формат
    void load(const std::string& filename);
    void save(const std::string& filename) const;

    // бинарный формат
    void loadBinary(const std::string& filename);
    void saveBinary(const std::string& filename) const;

    void execute(const std::string& query);

private:
    int       find(const std::string& name) const;
    DSRecord* add(const std::string& name, DSKind kind);

private:
    DSRecord recs[MAX_DS];
    int      count;
};

// =======================
// Реализация DBMS
// =======================

DBMS::DBMS()
    : count(0)
{
}

DBMS::~DBMS()
{
    clear();
}

void DBMS::clear()
{
    for (int i = 0; i < count; ++i) {
        switch (recs[i].kind) {
        case DSKind::ARRAY:
            delete static_cast<MyArray*>(recs[i].ptr);
            break;
        case DSKind::FLIST:
            delete static_cast<ForwardList*>(recs[i].ptr);
            break;
        case DSKind::LLIST:
            delete static_cast<List*>(recs[i].ptr);
            break;
        case DSKind::STACK:
            delete static_cast<Stack*>(recs[i].ptr);
            break;
        case DSKind::QUEUE:
            delete static_cast<Queue*>(recs[i].ptr);
            break;
        case DSKind::AVL:
            delete static_cast<AvlTree*>(recs[i].ptr);
            break;
        case DSKind::HCHAIN:
            delete static_cast<HashTable*>(recs[i].ptr);
            break;
        case DSKind::HOPEN:
            delete static_cast<HashTableOpen*>(recs[i].ptr);
            break;
        }
    }
    count = 0;
}

int DBMS::find(const std::string& name) const
{
    for (int i = 0; i < count; ++i) {
        if (recs[i].name == name) {
            return i;
        }
    }
    return -1;
}

DSRecord* DBMS::add(const std::string& name, DSKind kind)
{
    if (count >= MAX_DS) {
        return nullptr;
    }

    recs[count].name = name;
    recs[count].kind = kind;

    switch (kind) {
    case DSKind::ARRAY:
        recs[count].ptr = new MyArray();
        break;
    case DSKind::FLIST:
        recs[count].ptr = new ForwardList();
        break;
    case DSKind::LLIST:
        recs[count].ptr = new List();
        break;
    case DSKind::STACK:
        recs[count].ptr = new Stack();
        break;
    case DSKind::QUEUE:
        recs[count].ptr = new Queue();
        break;
    case DSKind::AVL:
        recs[count].ptr = new AvlTree();
        break;
    case DSKind::HCHAIN:
        recs[count].ptr = new HashTable();
        break;
    case DSKind::HOPEN:
        recs[count].ptr = new HashTableOpen();
        break;
    }

    ++count;
    return &recs[count - 1];
}

// =======================
// Текстовая сериализация
// Формат:
// TYPE NAME\n
// (данные serialize())
// END$\n
// =======================

void DBMS::load(const std::string& filename)
{
    clear();
    std::ifstream fin(filename);
    if (!fin) {
        return;
    }

    std::string line;
    std::string type;
    std::string name;
    std::string content;

    while (std::getline(fin, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream header(line);
        header >> type >> name;
        if (type.empty() || name.empty()) {
            continue;
        }

        content.clear();
        while (std::getline(fin, line) && line != "END$") {
            content += line;
            content.push_back('\n');
        }

        if (type == "ARRAY") {
            auto* arr = new MyArray();
            arr->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::ARRAY, arr};
        } else if (type == "FLIST") {
            auto* fl = new ForwardList();
            fl->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::FLIST, fl};
        } else if (type == "LLIST") {
            auto* ll = new List();
            ll->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::LLIST, ll};
        } else if (type == "STACK") {
            auto* st = new Stack();
            st->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::STACK, st};
        } else if (type == "QUEUE") {
            auto* q = new Queue();
            q->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::QUEUE, q};
        } else if (type == "AVL") {
            auto* avl = new AvlTree();
            avl->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::AVL, avl};
        } else if (type == "HCHAIN") {
            auto* ht = new HashTable();
            ht->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::HCHAIN, ht};
        } else if (type == "HOPEN") {
            auto* ho = new HashTableOpen();
            ho->deserialize(content);
            recs[count++] = DSRecord{name, DSKind::HOPEN, ho};
        }

        if (count >= MAX_DS) {
            break;
        }
    }
}

void DBMS::save(const std::string& filename) const
{
    std::ofstream fout(filename);
    if (!fout) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        std::string type;
        std::string data;

        switch (recs[i].kind) {
        case DSKind::ARRAY:
            type = "ARRAY";
            data = static_cast<MyArray*>(recs[i].ptr)->serialize();
            break;
        case DSKind::FLIST:
            type = "FLIST";
            data = static_cast<ForwardList*>(recs[i].ptr)->serialize();
            break;
        case DSKind::LLIST:
            type = "LLIST";
            data = static_cast<List*>(recs[i].ptr)->serialize();
            break;
        case DSKind::STACK:
            type = "STACK";
            data = static_cast<Stack*>(recs[i].ptr)->serialize();
            break;
        case DSKind::QUEUE:
            type = "QUEUE";
            data = static_cast<Queue*>(recs[i].ptr)->serialize();
            break;
        case DSKind::AVL:
            type = "AVL";
            data = static_cast<AvlTree*>(recs[i].ptr)->serialize();
            break;
        case DSKind::HCHAIN:
            type = "HCHAIN";
            data = static_cast<HashTable*>(recs[i].ptr)->serialize();
            break;
        case DSKind::HOPEN:
            type = "HOPEN";
            data = static_cast<HashTableOpen*>(recs[i].ptr)->serialize();
            break;
        }

        fout << type << ' ' << recs[i].name << '\n';
        fout << data;
        fout << "END$\n";
    }
}

// =======================
// Бинарная сериализация
//
// Формат файла:
//
// [u32 count]
//   повторить count раз:
//      [u8 kind]
//      [u32 nameLen] [name bytes]
//      [u32 dataLen] [data bytes]
//
// где data bytes — это то, что пишут serializeBinary()
// =======================

void DBMS::saveBinary(const std::string& filename) const
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return;
    }

    std::uint32_t cnt = static_cast<std::uint32_t>(count);
    out.write(reinterpret_cast<const char*>(&cnt), sizeof(cnt));

    for (int i = 0; i < count; ++i) {
        std::uint8_t kindByte = static_cast<std::uint8_t>(recs[i].kind);
        out.write(reinterpret_cast<const char*>(&kindByte), sizeof(kindByte));

        std::uint32_t nameLen =
            static_cast<std::uint32_t>(recs[i].name.size());
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        out.write(recs[i].name.data(), nameLen);

        std::ostringstream buf(std::ios::binary);

        switch (recs[i].kind) {
        case DSKind::ARRAY:
            static_cast<MyArray*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::FLIST:
            static_cast<ForwardList*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::LLIST:
            static_cast<List*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::STACK:
            static_cast<Stack*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::QUEUE:
            static_cast<Queue*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::AVL:
            static_cast<AvlTree*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::HCHAIN:
            static_cast<HashTable*>(recs[i].ptr)->serializeBinary(buf);
            break;
        case DSKind::HOPEN:
            static_cast<HashTableOpen*>(recs[i].ptr)->serializeBinary(buf);
            break;
        }

        const std::string bytes = buf.str();
        std::uint32_t     dataLen =
            static_cast<std::uint32_t>(bytes.size());
        out.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
        out.write(bytes.data(), dataLen);
    }
}

void DBMS::loadBinary(const std::string& filename)
{
    clear();
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return;
    }

    std::uint32_t cnt = 0;
    if (!in.read(reinterpret_cast<char*>(&cnt), sizeof(cnt))) {
        return;
    }

    for (std::uint32_t i = 0; i < cnt && count < MAX_DS; ++i) {
        std::uint8_t kindByte = 0;
        if (!in.read(reinterpret_cast<char*>(&kindByte), sizeof(kindByte))) {
            break;
        }
        DSKind kind = static_cast<DSKind>(kindByte);

        std::uint32_t nameLen = 0;
        if (!in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen))) {
            break;
        }
        std::string name(nameLen, '\0');
        if (!in.read(&name[0], nameLen)) {
            break;
        }

        std::uint32_t dataLen = 0;
        if (!in.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen))) {
            break;
        }
        std::string bytes(dataLen, '\0');
        if (!in.read(&bytes[0], dataLen)) {
            break;
        }

        std::istringstream buf(bytes, std::ios::binary);

        void* ptr = nullptr;
        switch (kind) {
        case DSKind::ARRAY: {
            auto* a = new MyArray();
            a->deserializeBinary(buf);
            ptr = a;
            break;
        }
        case DSKind::FLIST: {
            auto* f = new ForwardList();
            f->deserializeBinary(buf);
            ptr = f;
            break;
        }
        case DSKind::LLIST: {
            auto* l = new List();
            l->deserializeBinary(buf);
            ptr = l;
            break;
        }
        case DSKind::STACK: {
            auto* s = new Stack();
            s->deserializeBinary(buf);
            ptr = s;
            break;
        }
        case DSKind::QUEUE: {
            auto* q = new Queue();
            q->deserializeBinary(buf);
            ptr = q;
            break;
        }
        case DSKind::AVL: {
            auto* t = new AvlTree();
            t->deserializeBinary(buf);
            ptr = t;
            break;
        }
        case DSKind::HCHAIN: {
            auto* h = new HashTable();
            h->deserializeBinary(buf);
            ptr = h;
            break;
        }
        case DSKind::HOPEN: {
            auto* h = new HashTableOpen();
            h->deserializeBinary(buf);
            ptr = h;
            break;
        }
        }

        recs[count++] = DSRecord{name, kind, ptr};
    }
}

// =======================
// execute + автосейв
// =======================

void DBMS::execute(const std::string& query)
{
    std::string tokens[32];
    int         tokCount = 0;

    {
        std::istringstream iss(query);
        std::string        t;
        while (iss >> t && tokCount < 32) {
            tokens[tokCount++] = t;
        }
    }

    if (tokCount == 0) {
        return;
    }

    const std::string& cmd = tokens[0];

    auto autoSave = [this]() {
        this->save("db_autosave.txt");
        this->saveBinary("db_autosave.bin");
    };

    // ----------------- МАССИВ -----------------
    if (cmd == "MPUSH") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        MyArray* arr;
        if (idx == -1) arr = static_cast<MyArray*>(add(tokens[1], DSKind::ARRAY)->ptr);
        else           arr = static_cast<MyArray*>(recs[idx].ptr);
        arr->pushBack(tokens[2]);
        autoSave();
    } else if (cmd == "MINSERT") {
        if (tokCount < 4) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        MyArray* arr = static_cast<MyArray*>(recs[idx].ptr);
        int pos = std::stoi(tokens[2]);
        arr->insert(static_cast<std::size_t>(pos), tokens[3]);
        autoSave();
    } else if (cmd == "MDEL") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        MyArray* arr = static_cast<MyArray*>(recs[idx].ptr);
        int pos = std::stoi(tokens[2]);
        arr->removeAt(static_cast<std::size_t>(pos));
        autoSave();
    } else if (cmd == "MSET") {
        if (tokCount < 4) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        MyArray* arr = static_cast<MyArray*>(recs[idx].ptr);
        int pos = std::stoi(tokens[2]);
        arr->set(static_cast<std::size_t>(pos), tokens[3]);
        autoSave();
    } else if (cmd == "MGET") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        MyArray* arr = static_cast<MyArray*>(recs[idx].ptr);
        int pos = std::stoi(tokens[2]);
        try {
            std::cout << arr->at(static_cast<std::size_t>(pos)) << '\n';
        } catch (...) {
            std::cout << "<ERR>\n";
        }
    } else if (cmd == "MPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        MyArray* arr = static_cast<MyArray*>(recs[idx].ptr);
        arr->print();
    }

    // ----------- ОДНОСВЯЗНЫЙ СПИСОК -----------
    else if (cmd == "FPUSH") {
        if (tokCount < 4) return;
        int idx = find(tokens[1]);
        ForwardList* fl;
        if (idx == -1) fl = static_cast<ForwardList*>(add(tokens[1], DSKind::FLIST)->ptr);
        else           fl = static_cast<ForwardList*>(recs[idx].ptr);
        if (tokens[2] == "HEAD")      fl->pushFront(tokens[3]);
        else if (tokens[2] == "TAIL") fl->pushBack(tokens[3]);
        autoSave();
    } else if (cmd == "FDEL") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        if (tokens[2] == "HEAD") {
            fl->popFront();
        } else if (tokens[2] == "VAL" && tokCount > 3) {
            fl->removeByValue(tokens[3]);
        }
        autoSave();
    } else if (cmd == "FPUSH_AFTER" && tokCount >= 4) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        fl->insertAfter(tokens[2], tokens[3]);
        autoSave();
    } else if (cmd == "FPUSH_BEFORE" && tokCount >= 4) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        fl->insertBefore(tokens[2], tokens[3]);
        autoSave();
    } else if (cmd == "FDEL_AFTER" && tokCount >= 3) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        fl->removeAfter(tokens[2]);
        autoSave();
    } else if (cmd == "FDEL_BEFORE" && tokCount >= 3) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        fl->removeBefore(tokens[2]);
        autoSave();
    } else if (cmd == "FDEL_TAIL" && tokCount >= 2) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        fl->popBack();
        autoSave();
    } else if (cmd == "FPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        ForwardList* fl = static_cast<ForwardList*>(recs[idx].ptr);
        fl->print();
    }

    // ----------- ДВУСВЯЗНЫЙ СПИСОК -----------
    else if (cmd == "LPUSH") {
        if (tokCount < 4) return;
        int idx = find(tokens[1]);
        List* l;
        if (idx == -1) l = static_cast<List*>(add(tokens[1], DSKind::LLIST)->ptr);
        else           l = static_cast<List*>(recs[idx].ptr);
        if (tokens[2] == "HEAD")      l->pushFront(tokens[3]);
        else if (tokens[2] == "TAIL") l->pushBack(tokens[3]);
        autoSave();
    } else if (cmd == "LDEL") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        List* l = static_cast<List*>(recs[idx].ptr);
        if (tokens[2] == "HEAD")      l->popFront();
        else if (tokens[2] == "TAIL") l->popBack();
        else if (tokens[2] == "VAL" && tokCount > 3) {
            l->removeByValue(tokens[3]);
        }
        autoSave();
    } else if (cmd == "LPUSH_AFTER" && tokCount >= 4) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        List* l = static_cast<List*>(recs[idx].ptr);
        l->insertAfter(tokens[2], tokens[3]);
        autoSave();
    } else if (cmd == "LPUSH_BEFORE" && tokCount >= 4) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        List* l = static_cast<List*>(recs[idx].ptr);
        l->insertBefore(tokens[2], tokens[3]);
        autoSave();
    } else if (cmd == "LDEL_AFTER" && tokCount >= 3) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        List* l = static_cast<List*>(recs[idx].ptr);
        l->removeAfter(tokens[2]);
        autoSave();
    } else if (cmd == "LDEL_BEFORE" && tokCount >= 3) {
        int idx = find(tokens[1]);
        if (idx == -1) return;
        List* l = static_cast<List*>(recs[idx].ptr);
        l->removeBefore(tokens[2]);
        autoSave();
    } else if (cmd == "LPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        List* l = static_cast<List*>(recs[idx].ptr);
        l->print();
    }

    // ----------------- СТЕК -----------------
    else if (cmd == "SPUSH") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        Stack* s;
        if (idx == -1) s = static_cast<Stack*>(add(tokens[1], DSKind::STACK)->ptr);
        else           s = static_cast<Stack*>(recs[idx].ptr);
        s->push(tokens[2]);
        autoSave();
    } else if (cmd == "SPOP") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        Stack* s = static_cast<Stack*>(recs[idx].ptr);
        s->pop();
        autoSave();
    } else if (cmd == "SPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        Stack* s = static_cast<Stack*>(recs[idx].ptr);
        s->print();
    }

    // --------------- ОЧЕРЕДЬ ---------------
    else if (cmd == "QPUSH") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        Queue* q;
        if (idx == -1) q = static_cast<Queue*>(add(tokens[1], DSKind::QUEUE)->ptr);
        else           q = static_cast<Queue*>(recs[idx].ptr);
        q->push(tokens[2]);
        autoSave();
    } else if (cmd == "QPOP") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        Queue* q = static_cast<Queue*>(recs[idx].ptr);
        q->pop();
        autoSave();
    } else if (cmd == "QPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        Queue* q = static_cast<Queue*>(recs[idx].ptr);
        q->print();
    }

    // ------------- AVL-ДЕРЕВО -------------
    else if (cmd == "TINSERT") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        AvlTree* t;
        if (idx == -1) t = static_cast<AvlTree*>(add(tokens[1], DSKind::AVL)->ptr);
        else           t = static_cast<AvlTree*>(recs[idx].ptr);
        t->insert(tokens[2]);
        autoSave();
    } else if (cmd == "TDEL") {
        if (tokCount < 3) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        AvlTree* t = static_cast<AvlTree*>(recs[idx].ptr);
        t->remove(tokens[2]);
        autoSave();
    } else if (cmd == "TPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        AvlTree* t = static_cast<AvlTree*>(recs[idx].ptr);
        t->print();
    }

    // ---------- ХЕШ-Таблица ЦЕПНАЯ ----------
    else if (cmd == "HSET") {
        // HSET name key value...
        if (tokCount < 4) return;
        int idx = find(tokens[1]);
        HashTable* h;
        if (idx == -1) h = static_cast<HashTable*>(add(tokens[1], DSKind::HCHAIN)->ptr);
        else           h = static_cast<HashTable*>(recs[idx].ptr);

        const std::string& key = tokens[2];
        std::string        value = tokens[3];
        for (int i = 4; i < tokCount; ++i) {
            value.push_back(' ');
            value += tokens[i];
        }

        h->insert(key, value);
        autoSave();
    } else if (cmd == "HPRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        HashTable* h = static_cast<HashTable*>(recs[idx].ptr);
        h->print();
    }

    // ----- ХЕШ-Таблица ОТКР. АДРЕСАЦИЯ -----
    else if (cmd == "H2SET") {
        // H2SET name key value...
        if (tokCount < 4) return;
        int idx = find(tokens[1]);
        HashTableOpen* h;
        if (idx == -1) h = static_cast<HashTableOpen*>(add(tokens[1], DSKind::HOPEN)->ptr);
        else           h = static_cast<HashTableOpen*>(recs[idx].ptr);

        const std::string& key = tokens[2];
        std::string        value = tokens[3];
        for (int i = 4; i < tokCount; ++i) {
            value.push_back(' ');
            value += tokens[i];
        }

        h->insert(key, value);
        autoSave();
    } else if (cmd == "H2PRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        HashTableOpen* h = static_cast<HashTableOpen*>(recs[idx].ptr);
        h->print();
    }

    // --------- HELP / PRINT ---------
    else if (cmd == "HELP") {
        std::cout <<
            "МАССИВ (M): MPUSH name val | MINSERT name pos val | MDEL name pos | MSET name pos val | MGET name pos | MPRINT name\n"
            "ОДНОСВЯЗНЫЙ СПИСОК (F): FPUSH name HEAD/TAIL val | FDEL name HEAD/VAL val |\n"
            "                        FPUSH_AFTER name after val | FPUSH_BEFORE name before val |\n"
            "                        FDEL_AFTER name after | FDEL_BEFORE name before | FDEL_TAIL name | FPRINT name\n"
            "ДВУСВЯЗНЫЙ СПИСОК (L): LPUSH name HEAD/TAIL val | LDEL name HEAD/TAIL/VAL val |\n"
            "                        LPUSH_AFTER name after val | LPUSH_BEFORE name before val |\n"
            "                        LDEL_AFTER name after | LDEL_BEFORE name before | LPRINT name\n"
            "СТЕК (S): SPUSH name val | SPOP name | SPRINT name\n"
            "ОЧЕРЕДЬ (Q): QPUSH name val | QPOP name | QPRINT name\n"
            "AVL-ДЕРЕВО (T): TINSERT name val | TDEL name val | TPRINT name\n"
            "ХЕШ-ТАБЛИЦА цепная: HSET name key value... | HPRINT name\n"
            "ХЕШ-ТАБЛИЦА откр.: H2SET name key value... | H2PRINT name\n"
            "EXIT/QUIT — выход\n";
    } else if (cmd == "PRINT") {
        if (tokCount < 2) return;
        int idx = find(tokens[1]);
        if (idx == -1) return;
        switch (recs[idx].kind) {
        case DSKind::ARRAY:
            static_cast<MyArray*>(recs[idx].ptr)->print();
            break;
        case DSKind::FLIST:
            static_cast<ForwardList*>(recs[idx].ptr)->print();
            break;
        case DSKind::LLIST:
            static_cast<List*>(recs[idx].ptr)->print();
            break;
        case DSKind::STACK:
            static_cast<Stack*>(recs[idx].ptr)->print();
            break;
        case DSKind::QUEUE:
            static_cast<Queue*>(recs[idx].ptr)->print();
            break;
        case DSKind::AVL:
            static_cast<AvlTree*>(recs[idx].ptr)->print();
            break;
        case DSKind::HCHAIN:
            static_cast<HashTable*>(recs[idx].ptr)->print();
            break;
        case DSKind::HOPEN:
            static_cast<HashTableOpen*>(recs[idx].ptr)->print();
            break;
        }
    }
}

// =======================
// main: CLI
// =======================

int main()
{
    DBMS db;
    std::string line;

    // Автозагрузка:
    // если есть бинарь — считаем, что проект 1 (бинарный формат) приоритетен,
    // иначе пробуем текст.
    {
        std::ifstream bin("db_autosave.bin", std::ios::binary);
        if (bin) {
            db.loadBinary("db_autosave.bin");
        } else {
            std::ifstream txt("db_autosave.txt");
            if (txt) {
                db.load("db_autosave.txt");
            }
        }
    }

    while (std::getline(std::cin, line)) {
        if (line == "EXIT" || line == "QUIT") {
            break;
        }
        if (line.empty()) {
            continue;
        }
        db.execute(line);
    }

    return 0;
}
