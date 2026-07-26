// Compiled as C++17, so the standard library is available. extern "C" keeps the
// symbol names unmangled, which is what a Cypescript `declare` expects.
#include <string>
#include <vector>
#include <numeric>

extern "C" {

int cpp_sum_to(int n) {
    std::vector<int> values(n);
    std::iota(values.begin(), values.end(), 1);
    return std::accumulate(values.begin(), values.end(), 0);
}

const char *cpp_greet(const char *name) {
    std::string *result = new std::string("hello, " + std::string(name));
    return result->c_str();
}

}
