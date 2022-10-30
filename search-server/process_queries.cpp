#include "process_queries.h"
#include <algorithm>
#include <numeric>
#include <execution>
#include <iterator>
#include "log_duration.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> out(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), out.begin(),
        [&search_server](auto q) {return search_server.FindTopDocuments(q); });
    return out;
}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    std::vector<std::vector<Document>> vec_o_vec = ProcessQueries(search_server, queries);

    std::vector<Document> out;
    for (std::vector<Document>& v : vec_o_vec) {
        out.insert(out.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
    }

    return out;
}