#include "string_processing.h"
#include <iostream>
using namespace std;

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;
	for (const char c : text) {
		if (c == ' ') {
			if (!word.empty()) {
				words.push_back(word);
				word.clear();
			}
		}
		else {
			word += c;
		}
	}
	if (!word.empty()) {
		words.push_back(word);
	}

	return words;
}

//vector<string_view> SplitIntoWordsView(string_view str) {
//	vector<string_view> result;
//
//	size_t start = str.find_first_not_of(' ');
//	str.remove_prefix(min(str.size(), start));
//	while (!str.empty()) {
//		size_t space = str.find(" ");
//		size_t sym = str.find_first_not_of(' ', space);
//		result.push_back(str.substr(0, space));
//		str.remove_prefix(min(str.size(), sym));
//	}
//
//	return result;
//}

vector<string_view> SplitIntoWordsView(string_view str) {
	vector<string_view> result;
	while (true) {
		const auto space = str.find(' ');
		if (space != 0 && !str.empty()) {
			result.push_back(str.substr(0, space));
		}
		if (space == str.npos) {
			break;
		}
		else {
			str.remove_prefix(space + 1);
		}
	}
	return result;
}



