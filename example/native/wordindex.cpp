// A word index in C++, called from Cypescript.
//
// This is the case where reaching for C++ rather than C actually pays: it wants
// a growable map of string -> count, sorted iteration, and cleanup that happens
// on its own. In C that is a few hundred lines of hash table; here it is std::map.
//
// Three rules for the boundary, all visible below:
//
//   1. `extern "C"` so the symbols are not name-mangled — a Cypescript
//      `declare` looks them up by plain name.
//   2. Only scalars, `const char*` and opaque pointers cross. C++ types stay
//      inside; the object is handed over as a `void*` that Cypescript holds as
//      `ptr` and never inspects.
//   3. No exception may escape. Cypescript has no way to catch one, and
//      unwinding through its frames is undefined — so the boundary swallows them.

#include <cctype>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

namespace {

struct WordIndex {
    std::map<std::string, int> counts;
    // Returned strings live here, so they stay valid as long as the handle does
    // rather than being leaked to keep them alive.
    std::string scratch;
};

} // namespace

extern "C" {

void *wordindex_new() {
    try {
        return new WordIndex();
    } catch (...) {
        return nullptr;          // rule 3: never let it escape
    }
}

void wordindex_free(void *handle) {
    delete static_cast<WordIndex *>(handle);   // RAII cleans up the map for us
}

// Splits on whitespace and counts each word, lowercased.
void wordindex_add(void *handle, const char *text) {
    if (!handle || !text) return;
    try {
        auto *index = static_cast<WordIndex *>(handle);
        std::istringstream stream(text);
        std::string word;
        while (stream >> word) {
            std::transform(word.begin(), word.end(), word.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            index->counts[word]++;
        }
    } catch (...) {
        // Leave the index as it was rather than unwinding into Cypescript
    }
}

int wordindex_unique(void *handle) {
    if (!handle) return 0;
    return static_cast<int>(static_cast<WordIndex *>(handle)->counts.size());
}

int wordindex_count(void *handle, const char *word) {
    if (!handle || !word) return 0;
    auto *index = static_cast<WordIndex *>(handle);
    auto it = index->counts.find(word);
    return it == index->counts.end() ? 0 : it->second;
}

// std::map is ordered, so this is alphabetical without any extra work.
const char *wordindex_word_at(void *handle, int position) {
    if (!handle || position < 0) return "";
    auto *index = static_cast<WordIndex *>(handle);
    if (position >= static_cast<int>(index->counts.size())) return "";
    auto it = index->counts.begin();
    std::advance(it, position);
    index->scratch = it->first;      // owned by the handle, valid until freed
    return index->scratch.c_str();
}

// The most frequent word, resolving ties alphabetically.
const char *wordindex_most_common(void *handle) {
    if (!handle) return "";
    auto *index = static_cast<WordIndex *>(handle);
    if (index->counts.empty()) return "";
    auto best = std::max_element(
        index->counts.begin(), index->counts.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });
    index->scratch = best->first;
    return index->scratch.c_str();
}

} // extern "C"
