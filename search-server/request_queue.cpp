#include "request_queue.h"

	 RequestQueue::RequestQueue(const SearchServer& search_server)
		:search_server_(search_server)
	{
	}

	std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
		std::vector<Document> res = search_server_.FindTopDocuments(raw_query, status);
		AddRequest(res.size());
		return res;
	}

	std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
		std::vector<Document> res = search_server_.FindTopDocuments(raw_query);
		AddRequest(res.size());
		return res;
	}

	int RequestQueue::GetNoResultRequests() const {
		return empty_results_;
	}


		RequestQueue::QueryResult::QueryResult(int results, uint64_t time)
			:results_(results), time_(time)
		{

		}
		//std::vector<Document> result_;


	void RequestQueue::AddRequest(int results) {
		current_time_ == min_in_day_ - 1 ? current_time_ = 0 : ++current_time_;
		if (requests_.size() == min_in_day_) {
			if (requests_.front().results_ == 0) {
				--empty_results_;
			}
			requests_.pop_front();
		}
		requests_.push_back(QueryResult(results, current_time_));
		if (results == 0) {
			++empty_results_;
		}
	}
