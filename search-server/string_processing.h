#pragma once
#include <string>
#include <vector>
#include <set>
#include "document.h"
#include "paginator.h"

std::vector<std::string> SplitIntoWords(const std::string& text);


	template <typename StringContainer>
	std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer & strings) {
		std::set<std::string> non_empty_strings;
		for (const std::string& str : strings) {
			if (!str.empty()) {
				non_empty_strings.insert(str);
			}
		}
		return non_empty_strings;
	}

	void PrintDocument(const Document& document);

	void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);

	template <typename Iterator>
	std::ostream& operator<<(std::ostream& out, IteratorRange<Iterator> page) {

		for (auto it = page.begin(); it != page.end(); ++it) {
			PrintDocument(*it);
		}

		return out;
	}