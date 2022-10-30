#include "search_server.h"
#include "string_processing.h"
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <utility>
#include <mutex>
using namespace std;

SearchServer::SearchServer(const std::string& stop_words_text)
	: SearchServer(SplitIntoWords(stop_words_text))
{
}

SearchServer::SearchServer(const std::string_view stop_words_text)
	: SearchServer(SplitIntoWords(stop_words_text))
{
	stop_words_sv_ = set<string_view>(stop_words_.begin(), stop_words_.end());
}

void SearchServer::AddDocument(int document_id, const string_view document, DocumentStatus status, const vector<int>& ratings) {
	if ((document_id < 0) || (documents_.count(document_id) > 0)) {
		throw invalid_argument("invalid document id!"s);
	}
	vector<string> words;
	words = SplitIntoWordsNoStop(document);
	set<string> words_set;
	const double inv_word_count = 1.0 / words.size();
	for (string& word : words) {
		auto iter = words_set.insert(words_set.begin(), move(word));
		word_to_document_freqs_[string_view(*iter)][document_id] += inv_word_count;
		id_to_words_freqs_[document_id][string_view(*iter)] += inv_word_count;
	}

	documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status, move(words_set) });
	document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string_view raw_query, DocumentStatus status) const {
	return FindTopDocuments(
		raw_query,
		[status](int document_id, DocumentStatus document_status, int rating) {
			return document_status == status;
		});
}

vector<Document> SearchServer::FindTopDocuments(const string_view raw_query) const {
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
	return documents_.size();
}

tuple<SearchServer::DocText, DocumentStatus> SearchServer::MatchDocument(const string_view raw_query, int document_id) const {
	using namespace std::literals;
	if (!document_ids_.count(document_id)) {
		throw std::out_of_range("Wrong Id!"s);
	}

	Query query = ParseQuery(raw_query);
	vector<string_view> matched_words;
	const std::map<std::string_view, double>* const words = &id_to_words_freqs_.at(document_id);
	for (string_view word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if ((*words).count(word)) {
			matched_words.push_back(word);
		}
	}
	for (string_view word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			matched_words.clear();
			break;
		}
	}

	std::set<std::string_view> s(matched_words.begin(), matched_words.end());
	matched_words.assign(s.begin(), s.end());

	return { matched_words, documents_.at(document_id).status };
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string_view text) const {
	vector<string> words;
	for (string_view word : SplitIntoWords(text)) {
		if (!IsValidWord(word)) {
			throw invalid_argument("invalid document!"s);
		}
		if (!IsStopWord(word)) {
			words.push_back(std::string(word));
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

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {

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
	return QueryWord{ text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const string_view text_sv) const {
	Query q;

	std::vector<std::string_view> split = SplitIntoWords(text_sv);

	q.minus_words.reserve(split.size());
	q.plus_words.reserve(split.size());
	for (string_view word : split) {
		QueryWord query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				q.minus_words.push_back(move(query_word.data));
			}
			else {
				q.plus_words.push_back(move(query_word.data));
			}
		}
	}

	q.minus_words.shrink_to_fit();
	q.plus_words.shrink_to_fit();

	return q;
}

bool SearchServer::IsStopWord(const string_view word) const {
	return stop_words_sv_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
	return none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
		});
}
//--------------------------------------------------------------------------------

double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

std::set<int>::iterator SearchServer::begin() {
	return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end() {
	return document_ids_.end();
}

const std::map<std::string_view, double> SearchServer::GetWordFrequencies(int document_id) const {
	auto it = id_to_words_freqs_.find(document_id);
	std::map<std::string_view, double> res;
	if (it != id_to_words_freqs_.end()) {
		for (auto a = (*it).second.begin(); a != (*it).second.end(); ++a) {
			res.insert(*a);
		}
	}
	return res;
}

void SearchServer::RemoveDocument(int document_id) {
	for (auto [word, freq] : GetWordFrequencies(document_id)) {
		word_to_document_freqs_.at(word).erase(document_id);
	}
	id_to_words_freqs_.erase(document_id);
	document_ids_.erase(document_id);
	documents_.erase(document_id);
}

void SearchServer::RemoveDocument(const execution::sequenced_policy&,
	int document_id) {
	RemoveDocument(document_id);
}

 void SearchServer::RemoveDocument(const execution::parallel_policy&,
	 int document_id) {

		 auto temp = GetWordFrequencies(document_id);
	std::vector<std::pair<std::string_view, double>> words_of_doc(temp.begin(),
	temp.end());

	mutex m;

	for_each(
		execution::par,
 words_of_doc.begin(),
 words_of_doc.end(),
[&](auto& word) mutable {
			lock_guard<mutex> l(m);
			word_to_document_freqs_.at(word.first).erase(document_id); 
		}
);

	 id_to_words_freqs_.erase(document_id);
	 document_ids_.erase(document_id);
	 documents_.erase(document_id);
 }

