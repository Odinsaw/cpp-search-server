#pragma once
#include <string>
#include <vector>
#include <set>
#include <string_view>

	template <typename StringContainer>
	std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer & strings) {
		std::set<std::string> non_empty_strings;
		for (const auto& str : strings) {
			if (!str.empty()) {
				non_empty_strings.insert(std::string(str));
			}
		}
		return non_empty_strings;
	}

	std::vector<std::string_view> SplitIntoWords(std::string_view str);
