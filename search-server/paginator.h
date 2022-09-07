#pragma once

template <typename Iterator>
class IteratorRange {
public:
	IteratorRange() = default;
	IteratorRange(Iterator a, Iterator b)
		:p1(a), p2(b)
	{

	}
	auto begin() {
		return p1;
	}
	auto end() {
		return p2;
	}
	Iterator p1, p2;
};
template <typename Iterator>
class Paginator {
public:
	Paginator(Iterator container_begin, Iterator container_end, int page_size)
	{
		for (auto it = container_begin; it != container_end; distance(it, container_end) > page_size ? it += page_size : it = container_end) {
			Iterator it2;
			if (distance(it, container_end) > page_size) {
				it2 = it + page_size;
			}
			else {
				it2 = container_end;
			}
			pages_.push_back(IteratorRange<Iterator>(it, it2));
		}
	}
	auto begin() const {
		return pages_.begin();
	}
	auto end() const {
		return pages_.end();
	}
	std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
	return Paginator(begin(c), end(c), page_size);
}