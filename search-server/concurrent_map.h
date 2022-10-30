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
        :maps_(std::vector<map_and_lock_>(bucket_count))
    {
    }

    Access operator[](const Key& key) {
        auto map_index = static_cast<unsigned int>(key) % static_cast<unsigned int>(maps_.size());
        return { std::lock_guard<std::mutex>(maps_[map_index].mut), maps_[map_index].map[key] };
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> out;

        for (map_and_lock_& el : maps_) {
            std::lock_guard<std::mutex> lock(el.mut);
            for (const auto& [key, val] : el.map) {
                out.insert({ key,val });
            }
        }
        return out;
    }

    void Erase(const Key& key) {
        auto map_index = static_cast<unsigned int>(key) % static_cast<unsigned int>(maps_.size());
        std::lock_guard<std::mutex> lock(maps_[map_index].mut);
        maps_[map_index].map.erase(key);
    }

private:
    struct map_and_lock_ {
        std::mutex mut;
        std::map<Key, Value> map;
    };
    std::vector<map_and_lock_> maps_;
};