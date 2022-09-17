#include "search_server.h"
#include "string_processing.h"
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cmath>
using namespace std;

	SearchServer::SearchServer(const std::string& stop_words_text)
		: SearchServer(SplitIntoWords(stop_words_text)) 
	{

	}
	
	void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
		if ((document_id < 0) || (documents_.count(document_id) > 0)) {
			throw invalid_argument("invalid document id!"s);
		}
		vector<string> words;
		words = SplitIntoWordsNoStop(document);

		const double inv_word_count = 1.0 / words.size();
		for (const string& word : words) {
			word_to_document_freqs_[word][document_id] += inv_word_count;
			id_to_words_freqs_[document_id][word] += inv_word_count;
		}

		documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
		document_ids_.insert(document_id);
	}

	vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
		return FindTopDocuments(
			raw_query,
			[status](int document_id, DocumentStatus document_status, int rating) {
				return document_status == status;
			});
	}

	vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
		return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}

	int SearchServer::GetDocumentCount() const {
		return documents_.size();
	}

	/*int SearchServer::GetDocumentId(int index) const {
		if (index >= 0 && index < GetDocumentCount()) {
			return document_ids_[index];
		}
		throw out_of_range("invalid request"s);
	}*/

	tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
		Query query = ParseQuery(raw_query);
		vector<string> matched_words;
		for (const string& word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			if (word_to_document_freqs_.at(word).count(document_id)) {
				matched_words.push_back(word);
			}
		}
		for (const string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			if (word_to_document_freqs_.at(word).count(document_id)) {
				matched_words.clear();
				break;
			}
		}

		return { matched_words, documents_.at(document_id).status };
	}

	bool SearchServer::IsStopWord(const string& word) const {
		return stop_words_.count(word) > 0;
	}

	 bool SearchServer::IsValidWord(const std::string& word) {
		return none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			});
	}

	vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			if (!IsValidWord(word)) {
				throw invalid_argument("invalid document!"s);
			}
			if (!IsStopWord(word)) {
				words.push_back(word);
			}
		}
		return words;
	}

	int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
		if (ratings.empty()) {
			return 0;
		}
		int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
		return rating_sum / static_cast<int>(ratings.size());
	}

	SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {

		if (text.empty()) {
			throw invalid_argument("invalid query!"s);
		}
		bool is_minus = false;
		if (text[0] == '-') {
			is_minus = true;
			text = text.substr(1);
		}
		if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
			throw invalid_argument("invalid query!"s);
		}
		return QueryWord{ text, is_minus, IsStopWord(text) };;
	}

	SearchServer::Query SearchServer::ParseQuery(const string& text) const {
		Query q;
		for (const string& word : SplitIntoWords(text)) {
			QueryWord query_word = ParseQueryWord(word);
			if (!query_word.is_stop) {
				if (query_word.is_minus) {
					q.minus_words.insert(query_word.data);
				}
				else {
					q.plus_words.insert(query_word.data);
				}
			}
		}
		return q;
	}

	double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
		return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
	}

	std::set<int>::iterator SearchServer::begin() {
		return document_ids_.begin();
	}

	std::set<int>::iterator SearchServer::end() {
		return document_ids_.end();
	}

	const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
		auto it = id_to_words_freqs_.find(document_id);
		static std::map<std::string, double> null_res;
		return it == id_to_words_freqs_.end()? null_res : (*it).second;
	 }

	void SearchServer::RemoveDocument(int document_id) {
		for (auto [word,freq] : GetWordFrequencies(document_id))  {
				word_to_document_freqs_.at(word).erase(document_id);
	}
		id_to_words_freqs_.erase(document_id);
			document_ids_.erase(document_id);
			documents_.erase(document_id);
	}

