#include "request_queue.h"

	 RequestQueue::RequestQueue(const SearchServer& search_server)
		:search_server_(search_server)
	{
	}

	std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
		std::vector<Document> res = search_server_.FindTopDocuments(raw_query, status);
		AddRequest(res);
		return res;
	}

	std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
		std::vector<Document> res = search_server_.FindTopDocuments(raw_query);
		AddRequest(res);
		return res;
	}

	int RequestQueue::GetNoResultRequests() const {
		return empty_results_;
	}


		RequestQueue::QueryResult::QueryResult(const std::vector<Document>& res)
			:result_(res)
		{

		}
		std::vector<Document> result_;


	void RequestQueue::AddRequest(const std::vector<Document>& res) {
		time_ == min_in_day_ - 1 ? time_ = 0 : ++time_;
		if (requests_.size() == min_in_day_) {
			if (requests_.front().result_.empty()) {
				--empty_results_;
				requests_.front().result_;
			}
			requests_.pop_front();
		}
		requests_.push_back(QueryResult(res));
		if (res.empty()) {
			++empty_results_;
		}
	}
