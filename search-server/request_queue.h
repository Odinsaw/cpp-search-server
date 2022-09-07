#pragma once
#include <string>
#include <vector>
#include <deque>
#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer& search_server);

	template <typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
		std::vector<Document> res = search_server_.FindTopDocuments(raw_query, document_predicate);
		AddRequest(res);
		return res;
	}

	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

	std::vector<Document> AddFindRequest(const std::string& raw_query);

	int GetNoResultRequests() const;
private:
	struct QueryResult {
		QueryResult(const std::vector<Document>& res);
		std::vector<Document> result_;
	};
	const SearchServer& search_server_;
	int empty_results_ = 0;
	int time_ = 0;
	std::deque<QueryResult> requests_;
	const static int min_in_day_ = 1440;

	void AddRequest(const std::vector<Document>& res);
};
