#pragma once
#include "document.h"
#include "string_processing.h"
#include "log_duration.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <stdexcept>
#include <algorithm>
#include <execution>
#include <iterator>
#include <type_traits>
#include <string_view>
#include <cassert>
#include "concurrent_map.h"
const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double eps = 1e-6;

class SearchServer {

	typedef std::vector<std::string_view> DocText;

public:

	template <typename StringContainer>
	explicit SearchServer(const StringContainer& stop_words);

	explicit SearchServer(const std::string& stop_words_text);

	explicit SearchServer(const std::string_view& stop_words_text);

	void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const;
	std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const;
	std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const;

	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const;
	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentStatus status) const;
	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query) const;

	int GetDocumentCount() const;

	std::tuple<DocText, DocumentStatus> MatchDocument(const std::string_view& raw_query, int document_id) const;

	template< class ExecutionPolicy>
	std::tuple<DocText, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string_view& raw_query, int document_id) const;

	std::set<int>::iterator begin();

	std::set<int>::iterator end();

	const std::map<std::string_view, double> GetWordFrequencies(int document_id) const;

	void RemoveDocument(int document_id);

	template< class ExecutionPolicy>
	void RemoveDocument(ExecutionPolicy&& policy, int document_id);

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};
	const std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, DocumentData> documents_;
	std::set<int> document_ids_;
	std::map<int, std::map<std::string, double>> id_to_words_freqs_;

	bool IsStopWord(const std::string& word) const;

	static bool IsValidWord(const std::string& word);

	std::vector<std::string> SplitIntoWordsNoStop(const std::string_view& text) const;

	static int ComputeAverageRating(const std::vector<int>& ratings);

	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};

	//string_view--------------------------------------------
	std::set<std::string_view> stop_words_sv_;

	struct QueryWord_sv {
		std::string_view data;
		bool is_minus;
		bool is_stop;
	};

	struct Query_sv {
		std::vector<std::string_view> plus_words;
		std::vector<std::string_view> minus_words;
	};

	Query_sv ParseQuery_sv(const std::string_view& text) const;

	QueryWord_sv ParseQueryWord_sv(std::string_view& text) const;

	bool IsStopWord_sv(const std::string_view& word) const;

	static bool IsValidWord_sv(const std::string_view& word);

	//--------------------------------------------------------

	QueryWord ParseQueryWord(std::string text) const;

	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};

	Query ParseQuery(const std::string& text) const;

	double ComputeWordInverseDocumentFreq(const std::string& word) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query_sv& query, DocumentPredicate document_predicate) const;

	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const ExecutionPolicy& policy, const Query_sv& query, DocumentPredicate document_predicate) const;

};

template <typename StringContainer>
 SearchServer::SearchServer(const StringContainer& stop_words)
	: stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
	 stop_words_sv_ = std::set<std::string_view>(stop_words_.begin(), stop_words_.end());
	 using namespace std::literals;
	if (!std::all_of(stop_words_sv_.begin(), stop_words_sv_.end(), IsValidWord_sv)) {
		throw std::invalid_argument("invalid stop word!"s);
	}
}

 template <typename DocumentPredicate>
 std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const {
	 Query_sv query = ParseQuery_sv(raw_query);
	 auto matched_documents = FindAllDocuments(query, document_predicate);

	 sort(matched_documents.begin(), matched_documents.end(), [](const Document & lhs, const Document & rhs) {
		 if (std::abs(lhs.relevance - rhs.relevance) < eps) {
			 return lhs.rating > rhs.rating;
		 }
		 else {
			 return lhs.relevance > rhs.relevance;
		 }
		 });
	 if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		 matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	 }

	 return matched_documents;
 }

 template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query_sv& query, DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;

	std::set<std::string_view> plus_words(std::make_move_iterator(query.plus_words.begin()), std::make_move_iterator(query.plus_words.end()));

    for (const std::string_view& word : plus_words) {
   	 if (word_to_document_freqs_.count(std::string(word)) == 0) {
   		 continue;
   	 }
   	 const double inverse_document_freq = ComputeWordInverseDocumentFreq(std::string(word));
   	 for (const auto [document_id, term_freq] : word_to_document_freqs_.at(std::string(word))) {
   		 const auto& document_data = documents_.at(document_id);
   		 if (document_predicate(document_id, document_data.status, document_data.rating)) {
   			 document_to_relevance[document_id] += term_freq * inverse_document_freq;
   		 }
   	 }
    }

    for (const std::string_view& word : query.minus_words) {
   	 if (word_to_document_freqs_.count(std::string(word)) == 0) {
   		 continue;
   	 }
   	 for (const auto [document_id, _] : word_to_document_freqs_.at(std::string(word))) {
   		 document_to_relevance.erase(document_id);
   	 }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
   	 matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

 template< typename ExecutionPolicy>
 void SearchServer::RemoveDocument(ExecutionPolicy&& policy,
	 int document_id) {
		 if (std::is_same_v<ExecutionPolicy,
			 const std::execution::sequenced_policy&>) {
			 RemoveDocument(document_id);
			 return;
		 }
		 auto temp = GetWordFrequencies(document_id);
	std::vector<std::pair<std::string_view, double>> words_of_doc(temp.begin(),
    temp.end());

	std::for_each(
 policy,
 words_of_doc.begin(),
 words_of_doc.end(),
[&](auto& word) mutable {word_to_document_freqs_.at(static_cast<std::string>(word.first)).erase(document_id); }
);

	 id_to_words_freqs_.erase(document_id);
	 document_ids_.erase(document_id);
	 documents_.erase(document_id);
 }

 template< class ExecutionPolicy>
 std::tuple<SearchServer::DocText, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, const std::string_view& raw_query, int document_id) const {
	 
	 using namespace std::literals;

	 if (!document_ids_.count(document_id)) {
		 throw std::out_of_range("Wrong Id!"s);
	 }

	 if (std::is_same_v<ExecutionPolicy, const std::execution::sequenced_policy&>) {
		 return MatchDocument(raw_query, document_id);
	 }

	 const std::map<std::string, double>* const words = &id_to_words_freqs_.at(document_id);

	 Query_sv query = ParseQuery_sv(raw_query);
	 
	 std::vector<std::string> minus_words(query.minus_words.begin(), query.minus_words.end());

		 if (minus_words.end() != std::find_if(
			 policy,
			 minus_words.begin(),
			 minus_words.end(),
			 [&](std::string& word) {
				 return (*words).count(word);
			 }
		 )) {
			 return { {}, documents_.at(document_id).status };
		 }


	 std::vector<std::string_view> matched_words(query.plus_words.size());
	 std::transform(
		 policy,
		 query.plus_words.begin(),
		 query.plus_words.end(),
		 matched_words.begin(),
		 [&](std::string_view& word) {
			 auto it = (*words).find(std::string(word));
			 if (it != (*words).end()) {
				 return std::string_view((*it).first);
			 }
			 return ""sv;
			 ; }
	 );

	 std::set<std::string_view> s(std::make_move_iterator(matched_words.begin()), std::make_move_iterator(matched_words.end()));
	 s.erase(""sv);
	 matched_words.assign(std::make_move_iterator(s.begin()), std::make_move_iterator(s.end()));

	 return { matched_words, documents_.at(document_id).status };

 }
 
 template <typename ExecutionPolicy, typename DocumentPredicate>
 std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const {
	 Query_sv query = ParseQuery_sv(raw_query);
	 auto matched_documents = FindAllDocuments(policy, query, document_predicate);

	 std::sort(policy, matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
		 if (std::abs(lhs.relevance - rhs.relevance) < eps) {
			 return lhs.rating > rhs.rating;
		 }
		 else {
			 return lhs.relevance > rhs.relevance;
		 }
		 });
	 if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		 matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	 }

	 return matched_documents;
 }
 template <typename ExecutionPolicy>
 std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query, DocumentStatus status) const {
	 return FindTopDocuments(
		 policy,
		 raw_query,
		 [status](int document_id, DocumentStatus document_status, int rating) {
			 return document_status == status;
		 });
 }
 template <typename ExecutionPolicy>
 std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, const std::string_view& raw_query) const {
	 return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
 }

 template <typename ExecutionPolicy, typename DocumentPredicate>
 std::vector<Document> SearchServer::FindAllDocuments(const ExecutionPolicy& policy, const Query_sv& query, DocumentPredicate document_predicate) const {
	 ConcurrentMap<int, double> document_to_relevance(7);

	 std::set<std::string_view> plus_words_raw(std::make_move_iterator(query.plus_words.begin()), std::make_move_iterator(query.plus_words.end()));
	 std::vector<std::string_view> plus_words(std::make_move_iterator(plus_words_raw.begin()), std::make_move_iterator(plus_words_raw.end()));

	 auto func = [&](const std::string_view& word) {
		 const double inverse_document_freq = ComputeWordInverseDocumentFreq(std::string(word));
		 std::for_each(
			 policy,
			 word_to_document_freqs_.at(std::string(word)).begin(),
			 word_to_document_freqs_.at(std::string(word)).end(),
			 [&](const std::pair<int,double>& el) {
				 const auto& document_data = documents_.at(el.first);
				 if (document_predicate(el.first, document_data.status, document_data.rating)) {
					 document_to_relevance[el.first].ref_to_value += el.second * inverse_document_freq;
				 }
			 }
		 );
	 };

	 std::for_each(
	 policy,
		 plus_words.begin(),
		 plus_words.end(),
		 [&](const std::string_view& word) {
			 if (word_to_document_freqs_.count(std::string(word)) != 0) {
				 func(word);
			 }
		 }
	 );

	 auto eraser = [&](const std::string_view word) {
		 std::for_each(
			 policy,
			 word_to_document_freqs_.at(std::string(word)).begin(),
			 word_to_document_freqs_.at(std::string(word)).end(),
			 [&](const std::pair<int, double> el) {
				 document_to_relevance.Erase(el.first);
			 }
		 );
	 };

	 std::vector<std::string_view> minus_words(std::make_move_iterator(query.minus_words.begin()), std::make_move_iterator(query.minus_words.end()));


	 std::for_each(
		 policy,
		 minus_words.begin(),
		 minus_words.end(),
		 [&](const std::string_view& word) {
			  if (word_to_document_freqs_.count(std::string(word)) != 0) {
				  eraser(word);
			  }
		 }
		 );

	 std::map<int, double> document_to_relevance_ordinary = document_to_relevance.BuildOrdinaryMap();

	 std::vector<Document> matched_documents;
	 matched_documents.reserve(document_to_relevance_ordinary.size());
	 for (const auto [document_id, relevance] : document_to_relevance_ordinary) {
		 matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	 }

	 return matched_documents;
 }
