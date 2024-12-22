#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <utility>
#include <string>
#include <cassert>

unsigned long hash_djb2(const std::string& str) {
    unsigned long hash = 5381; // ��������� �������� ����
    for (char c : str) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
        // �������� ���� hash �� 5 ������� ����� � ��������� � ���� ������ ����,
        // ��� ������������ ��������� �� 33, ����� ��������� ������� ������ ������
    }
    return hash; // ���������� ����������� �������� ����
}

// ��������� ����� HashTable, ������� ��������� ���� �����, �������� � ���-�������
template <typename Key, typename Value, typename HashFunc = std::hash<Key>>
class HashTable {
public:
    // ����������� � ������������ ������� ���������� ������� ������� � ���-�������
    HashTable(size_t size = 100, HashFunc hash_func = HashFunc())
        : table(size), hash_function(hash_func), current_size(0), max_load_factor(0.7),min_load_factor (0.2) {}
    // �������������� ������� ��������� �������, ���-�������, ������� ������ � ������������ ����������� ����������

    // ������� �������� � �������
    void insert(const Key& key, const Value& value) {
        if (current_size >= table.size() * max_load_factor) {
            rehash(); // ���� ������� ������ ��������� ������������ ����������� ����������, ���������� �������
        }
        size_t index = hash_function(key) % table.size(); // ��������� ������ ��� �������
        table[index].emplace_back(key, value); // ��������� ���� ����-�������� � ��������������� ������
        current_size++; // ����������� ������� ������
    }

    // ����� �������� � �������
    bool find(const Key& key, Value& value) const {
        size_t index = hash_function(key) % table.size(); // ��������� ������ ��� ������
        for (const auto& pair : table[index]) { // �������� �� ������ � ������
            if (pair.first == key) { // ���� ���� ���������
                value = pair.second; // ���������� ��������
                return true; // ���������� true
            }
        }
        return false; // ���� ���� �� ������, ���������� false
    }

    // �������� �������� �� �������
    void remove(const Key& key) {
        size_t index = hash_function(key) % table.size(); // ��������� ������ ��� ��������
        auto& cell = table[index]; // �������� ������ �� ������ � ������
        cell.remove_if([&](const std::pair<Key, Value>& pair) { // ������� �������, ���� ���� ���������
            return pair.first == key;
            });
        current_size--; // ��������� ������� ������
        if (current_size < table.size() * min_load_factor) {
            rehash_down(); // ���� ������� ������ ������ ������������ ������������ ����������, ���������� ������� ����
        }
    }

    // ��������� �������� ������� �������
    size_t size() const {
        return current_size; // ���������� ������� ������
    }

    // ��������� �������� ������������ ����������
    double load_factor() const {
        return static_cast<double>(current_size) / table.size(); // ���������� ����������� ����������
    }

private:
    std::vector<std::list<std::pair<Key, Value>>> table; // ������ ������� ��� �������� ���������
    HashFunc hash_function; // ���-�������
    size_t current_size; // ������� ���������� ��������� � �������
    double max_load_factor; // ������������ ����������� ����������
    double min_load_factor; // ����������� ����������� ����������

    // ������������� �������
    void rehash() {
        size_t new_size = table.size() * 2; // ��������� ������ �������
        std::vector<std::list<std::pair<Key, Value>>> new_table(new_size); // ������� ����� �������
        for (const auto& cell : table) { // �������� �� ���� ������� ������ �������
            for (const auto& pair : cell) { // �������� �� ���� ��������� � ������
                size_t index = hash_function(pair.first) % new_size; // ��������� ����� ������
                new_table[index].emplace_back(pair.first, pair.second); // ��������� ������� � ����� �������
            }
        }
        table = std::move(new_table); // ���������� ����� ������� � ������
    }

    // ������������� ������� ��� ���������� �������
    void rehash_down() {
        size_t new_size = table.size() / 2; // ��������� ������ ������� �����
        std::vector<std::list<std::pair<Key, Value>>> new_table(new_size); // ������� ����� �������
        for (const auto& cell : table) { // �������� �� ���� ������� ������ �������
            for (const auto& pair : cell) { // �������� �� ���� ��������� � ������
                size_t index = hash_function(pair.first) % new_size; // ��������� ����� ������
                new_table[index].emplace_back(pair.first, pair.second); // ��������� ������� � ����� �������
            }
        }
        table = std::move(new_table); // ���������� ����� ������� � ������
    }
};


void test() {
    // ����� ��� ��������� ������
    HashTable<std::string, long long, std::function<unsigned long(const std::string&)>> hashTable(100, hash_djb2);

    // ������� ���������
    hashTable.insert("key1", 1LL);
    hashTable.insert("key2", 2LL);

    // �������� �������
    assert(hashTable.size() == 2);

    // ����� ��������
    long long value;
    assert(hashTable.find("key1", value));
    assert(value == 1LL);

    assert(hashTable.find("key2", value));
    assert(value == 2LL);

    // �������� ��������
    hashTable.remove("key1");
    assert(hashTable.size() == 1);
    assert(!hashTable.find("key1", value));

    // ��������� ������� � ��������
    hashTable.insert("key1", 1LL);
    assert(hashTable.size() == 2);
    assert(hashTable.find("key1", value));
    assert(value == 1LL);

    // �������� �������������
    for (long long i = 3; i <= 150; ++i) {
        hashTable.insert("key" + std::to_string(i), i);
    }
    assert(hashTable.size() == 150);
    assert(hashTable.load_factor() <= 0.7);

    // �������� ������������� ����
    for (long long i = 3; i <= 150; ++i) {
        hashTable.remove("key" + std::to_string(i));
    }
    assert(hashTable.size() == 2);
    assert(hashTable.load_factor() >= 0.2);

    // �������� ���-�������
    assert(hash_djb2("key1") == 2090432895);

    // �������� ���-������� ��� ������ ������
    assert(hash_djb2("") == 5381);

    // �������� ���-������� ��� �������� ������
    std::string test_string = "test_string";
    assert(hash_djb2(test_string) == 4175666075);

    // ����� ��� �������� ������
    HashTable<long long, std::string> intHashTable;

    // ������� ���������
    intHashTable.insert(1LL, "one");
    intHashTable.insert(2LL, "two");

    // �������� �������
    assert(intHashTable.size() == 2);

    // ����� ��������
    std::string str_value;
    assert(intHashTable.find(1LL, str_value));
    assert(str_value == "one");

    assert(intHashTable.find(2LL, str_value));
    assert(str_value == "two");

    // �������� ��������
    intHashTable.remove(1LL);
    assert(intHashTable.size() == 1);
    assert(!intHashTable.find(1LL, str_value));

    // ��������� ������� � ��������
    intHashTable.insert(1LL, "one");
    assert(intHashTable.size() == 2);
    assert(intHashTable.find(1LL, str_value));
    assert(str_value == "one");

    // �������� ���-������� ��� ����� 1
    std::hash<long long> int_hash;
    size_t hash_value = int_hash(1LL);


    std::cout << "All tests complete" << std::endl;
}



