#include "util.h"
#include <ctime>
#include <sstream>
#include <iomanip>

std::string nowStr(const char *fmt) {
	time_t now = std::time(nullptr);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&now), fmt);
	return ss.str();
}
