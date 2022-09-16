#include "remove_duplicates.h"
#include "log_duration.h"


void RemoveDuplicates(SearchServer& search_server) {

	std::set<int> queue_to_remove;

	for (auto it = search_server.begin(); it != search_server.end(); ++it)
	{
		if (queue_to_remove.count(*it)) {
			continue;
		}
		for (auto it1 = it + 1; it1 != search_server.end(); ++it1) {
			if (queue_to_remove.count(*it1)) {
				continue;
			}
			else if (IsDuplicate(search_server, *it, *it1)) {
				queue_to_remove.insert(*it1);
			}
		}
	}
	for (auto id : queue_to_remove) {
		search_server.RemoveDocument(id);
		std::cout << "Found duplicate document id "s << id << std::endl;
	}
}

bool IsDuplicate(SearchServer& search_server, int id1, int id2){

	auto doc1 = search_server.GetWordFrequencies(id1);
	auto doc2 = search_server.GetWordFrequencies(id2);

	return doc1.size() == doc2.size()
		&& std::equal(doc1.begin(), doc1.end(), doc2.begin(),
			[](auto a, auto b) { return a.first == b.first; });

}
