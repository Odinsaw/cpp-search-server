#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION1(x,stream) LogDuration UNIQUE_VAR_NAME_PROFILE(x,stream)
#define LOG_DURATION2(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define GET_MACRO(_1,_2,NAME,...) NAME
#define LOG_DURATION(...) GET_MACRO(__VA_ARGS__, LOG_DURATION1, LOG_DURATION2)(__VA_ARGS__)

class LogDuration {
public:
	using Clock = std::chrono::steady_clock;

	LogDuration(const std::string& id, std::ostream& stream = std::cerr)
		: id_(id), stream_(stream)
	{
	}

	~LogDuration() {
		using namespace std::chrono;
		using namespace std::literals;

		const auto end_time = Clock::now();
		const auto dur = end_time - start_time_;
		//stream_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
		stream_ << "Operation time: "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
	}

private:
	const std::string id_;
	std::ostream& stream_;
	const Clock::time_point start_time_ = Clock::now();
};
