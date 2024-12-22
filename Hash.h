#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <utility>
#include <string>
#include <cassert>

unsigned long hash_djb2(const std::string& str) {
    unsigned long hash = 5381; // Начальное значение хеша
    for (char c : str) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
        // Сдвигаем биты hash на 5 позиций влево и добавляем к нему самого себя,
        // что эквивалентно умножению на 33, затем добавляем текущий символ строки
    }
    return hash; // Возвращаем вычисленное значение хеша
}

// Шаблонный класс HashTable, который принимает типы ключа, значения и хеш-функции
template <typename Key, typename Value, typename HashFunc = std::hash<Key>>
class HashTable {
public:
    // Конструктор с возможностью задания начального размера таблицы и хеш-функции
    HashTable(size_t size = 100, HashFunc hash_func = HashFunc())
        : table(size), hash_function(hash_func), current_size(0), max_load_factor(0.7),min_load_factor (0.2) {}
    // Инициализируем таблицу заданного размера, хеш-функцию, текущий размер и максимальный коэффициент заполнения

    // Вставка элемента в таблицу
    void insert(const Key& key, const Value& value) {
        if (current_size >= table.size() * max_load_factor) {
            rehash(); // Если текущий размер превышает максимальный коэффициент заполнения, рехешируем таблицу
        }
        size_t index = hash_function(key) % table.size(); // Вычисляем индекс для вставки
        table[index].emplace_back(key, value); // Вставляем пару ключ-значение в соответствующий список
        current_size++; // Увеличиваем текущий размер
    }

    // Поиск элемента в таблице
    bool find(const Key& key, Value& value) const {
        size_t index = hash_function(key) % table.size(); // Вычисляем индекс для поиска
        for (const auto& pair : table[index]) { // Проходим по списку в ячейке
            if (pair.first == key) { // Если ключ совпадает
                value = pair.second; // Записываем значение
                return true; // Возвращаем true
            }
        }
        return false; // Если ключ не найден, возвращаем false
    }

    // Удаление элемента из таблицы
    void remove(const Key& key) {
        size_t index = hash_function(key) % table.size(); // Вычисляем индекс для удаления
        auto& cell = table[index]; // Получаем ссылку на список в ячейке
        cell.remove_if([&](const std::pair<Key, Value>& pair) { // Удаляем элемент, если ключ совпадает
            return pair.first == key;
            });
        current_size--; // Уменьшаем текущий размер
        if (current_size < table.size() * min_load_factor) {
            rehash_down(); // Если текущий размер меньше минимального коэффициента заполнения, рехешируем таблицу вниз
        }
    }

    // Получение текущего размера таблицы
    size_t size() const {
        return current_size; // Возвращаем текущий размер
    }

    // Получение текущего коэффициента заполнения
    double load_factor() const {
        return static_cast<double>(current_size) / table.size(); // Возвращаем коэффициент заполнения
    }

private:
    std::vector<std::list<std::pair<Key, Value>>> table; // Вектор списков для хранения элементов
    HashFunc hash_function; // Хеш-функция
    size_t current_size; // Текущее количество элементов в таблице
    double max_load_factor; // Максимальный коэффициент заполнения
    double min_load_factor; // Минимальный коэффициент заполнения

    // Рехеширование таблицы
    void rehash() {
        size_t new_size = table.size() * 2; // Удваиваем размер таблицы
        std::vector<std::list<std::pair<Key, Value>>> new_table(new_size); // Создаем новую таблицу
        for (const auto& cell : table) { // Проходим по всем ячейкам старой таблицы
            for (const auto& pair : cell) { // Проходим по всем элементам в ячейке
                size_t index = hash_function(pair.first) % new_size; // Вычисляем новый индекс
                new_table[index].emplace_back(pair.first, pair.second); // Вставляем элемент в новую таблицу
            }
        }
        table = std::move(new_table); // Перемещаем новую таблицу в старую
    }

    // Рехеширование таблицы для уменьшения размера
    void rehash_down() {
        size_t new_size = table.size() / 2; // Уменьшаем размер таблицы вдвое
        std::vector<std::list<std::pair<Key, Value>>> new_table(new_size); // Создаем новую таблицу
        for (const auto& cell : table) { // Проходим по всем ячейкам старой таблицы
            for (const auto& pair : cell) { // Проходим по всем элементам в ячейке
                size_t index = hash_function(pair.first) % new_size; // Вычисляем новый индекс
                new_table[index].emplace_back(pair.first, pair.second); // Вставляем элемент в новую таблицу
            }
        }
        table = std::move(new_table); // Перемещаем новую таблицу в старую
    }
};


void test() {
    // Тесты для строковых ключей
    HashTable<std::string, long long, std::function<unsigned long(const std::string&)>> hashTable(100, hash_djb2);

    // Вставка элементов
    hashTable.insert("key1", 1LL);
    hashTable.insert("key2", 2LL);

    // Проверка вставки
    assert(hashTable.size() == 2);

    // Поиск элемента
    long long value;
    assert(hashTable.find("key1", value));
    assert(value == 1LL);

    assert(hashTable.find("key2", value));
    assert(value == 2LL);

    // Удаление элемента
    hashTable.remove("key1");
    assert(hashTable.size() == 1);
    assert(!hashTable.find("key1", value));

    // Повторная вставка и проверка
    hashTable.insert("key1", 1LL);
    assert(hashTable.size() == 2);
    assert(hashTable.find("key1", value));
    assert(value == 1LL);

    // Проверка рехеширования
    for (long long i = 3; i <= 150; ++i) {
        hashTable.insert("key" + std::to_string(i), i);
    }
    assert(hashTable.size() == 150);
    assert(hashTable.load_factor() <= 0.7);

    // Проверка рехеширования вниз
    for (long long i = 3; i <= 150; ++i) {
        hashTable.remove("key" + std::to_string(i));
    }
    assert(hashTable.size() == 2);
    assert(hashTable.load_factor() >= 0.2);

    // Проверка хеш-функции
    assert(hash_djb2("key1") == 2090432895);

    // Проверка хеш-функции для пустой строки
    assert(hash_djb2("") == 5381);

    // Проверка хеш-функции для тестовой строки
    std::string test_string = "test_string";
    assert(hash_djb2(test_string) == 4175666075);

    // Тесты для числовых ключей
    HashTable<long long, std::string> intHashTable;

    // Вставка элементов
    intHashTable.insert(1LL, "one");
    intHashTable.insert(2LL, "two");

    // Проверка вставки
    assert(intHashTable.size() == 2);

    // Поиск элемента
    std::string str_value;
    assert(intHashTable.find(1LL, str_value));
    assert(str_value == "one");

    assert(intHashTable.find(2LL, str_value));
    assert(str_value == "two");

    // Удаление элемента
    intHashTable.remove(1LL);
    assert(intHashTable.size() == 1);
    assert(!intHashTable.find(1LL, str_value));

    // Повторная вставка и проверка
    intHashTable.insert(1LL, "one");
    assert(intHashTable.size() == 2);
    assert(intHashTable.find(1LL, str_value));
    assert(str_value == "one");

    // Проверка хеш-функции для числа 1
    std::hash<long long> int_hash;
    size_t hash_value = int_hash(1LL);


    std::cout << "All tests complete" << std::endl;
}



