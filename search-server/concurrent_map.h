#pragma once
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>

#include "log_duration.h"
//#include "test_framework.h"

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
        std::lock_guard<std::mutex> lock;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
        :maps_(std::vector<std::map<Key, Value>>(bucket_count)), ms_(std::vector<std::mutex>(bucket_count))
    {
    }

    Access operator[](const Key& key) {
        auto map_index = static_cast<unsigned int>(key) % static_cast<unsigned int>(maps_.size());
        return { std::lock_guard<std::mutex>(ms_[map_index]), maps_[map_index][key] };
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        //std::lock_guard<std::mutex> lock(m);
        std::map<Key, Value> out;

        for (unsigned long i = 0; i < maps_.size(); ++i) {
            std::lock_guard<std::mutex> lock(ms_[i]);
            for (const auto& [key, val] : maps_[i]) {
                out.insert({ key,val });
            }
        }
        return out;
    }

    void Erase(const Key& key) {
        auto map_index = static_cast<unsigned int>(key) % static_cast<unsigned int>(maps_.size());
        std::lock_guard<std::mutex> lock(ms_[map_index]);
        maps_[map_index].erase(key);
    }

private:
    std::vector<std::map<Key, Value>> maps_;
    std::vector<std::mutex> ms_;
    std::mutex m;
};
