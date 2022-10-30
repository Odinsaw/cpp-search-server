#include "string_processing.h"
#include <iostream>
using namespace std;

vector<string_view> SplitIntoWords(string_view str) {
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



