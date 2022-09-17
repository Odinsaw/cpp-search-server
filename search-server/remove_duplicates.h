#pragma once
#include "search_server.h"

void RemoveDuplicates(SearchServer& search_server);

template <typename key, typename value>
std::set<key> GetKeys(const std::map<key, value>& in);