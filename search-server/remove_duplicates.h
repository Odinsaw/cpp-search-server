#pragma once
#include "search_server.h"
void RemoveDuplicates(SearchServer& search_server);

bool IsDuplicate(SearchServer& search_server, int id1, int id2);