#include "remove_duplicates.h"
#include "log_duration.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {

	set<set<string_view>> words_set;
	set<int> delete_list;
	for (auto doc_id : search_server) {
		set<string_view> words = GetKeys(search_server.GetWordFrequencies(doc_id));
		if (words_set.find(words) != words_set.end()) {
			delete_list.insert(doc_id);
		}
		else {
			words_set.insert(words);
		}
	}

	for (auto id : delete_list) {
		std::cout << "Found duplicate document id "s << id << std::endl;
		search_server.RemoveDocument(id);
	}

}

template <typename key, typename value>
set<key> GetKeys(const map<key, value>& in) {
	set<key> out;
	for (auto [k, v] : in) {
		out.insert(k);
	}
	return out;
}