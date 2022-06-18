#include "../include/porter2.h"

namespace porter2 {
    namespace english {
        bool contains_vowel(const std::string &s, const int &start, const int &end) {
            for (int i = start; i < end; ++i) {
                if (is_vowel(s[i])) {
                    return true;
                }
            }

            return false;
        }
        bool if_contains_vowel_replace_end(std::string &s, const std::string &from, const std::string &to) {
            auto length = s.length();
            if (from.length() > length) return false;
            return length > 2 && contains_vowel(s, 0, s.length() - from.length()) && replace_end(s, from, to);
        }
        bool if_contains_vowel_replace_end(std::string &s, const int &end, const std::string &from, const std::string &to) {
            auto length = s.length();
            if (from.length() > length) return false;
            return length > 2 && contains_vowel(s, 0, end) && replace_end(s, from, to);
        }
        bool ends_with_double(const std::string &s) {
            auto length = s.length();
            return length > 2 && is_double_suffix(s);
        }
        int first_non_vowel(const std::string &s, const int &start) {
            auto length = s.length();

            for (int i = start; i < s.length(); ++i) {
                if (is_vowel(s[i - 1]) && !is_vowel(s[i])) {
                    return i + 1;
                }
            }

            return length; //the end of the word
        }
        bool is_double_suffix(const std::string &s) {
            const static std::string suffixes[] = {
                    "bb",
                    "dd",
                    "ff",
                    "gg",
                    "mm",
                    "nn",
                    "pp",
                    "rr",
                    "tt",
            };
            const static auto suffixes_size = 9;

            for (int i = 0; i < suffixes_size; ++i) {
                if (ends_with(s, suffixes[i])) {
                    return true;
                }
            }

            return false;
        }

        bool is_vowel(const char &c) {
            return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y';
        }
        bool is_vowel_without_y(const char &c) {
            return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
        }
        bool is_valid_li_ending(const char &c) {
            return c == 'c' || c == 'd' || c == 'e' || c == 'g' || c == 'h' || c == 'k' || c == 'm' || c == 'n' || c == 'r' || c == 't';
        }
        bool is_short(const std::string &s) {
            auto length = s.length();

            bool first = length >= 3 && !is_vowel(s[length - 3]) && is_vowel(s[length - 2]) && !is_vowel(s[length - 1])
                         &&
                         s[length - 1] != 'w' && s[length - 1] != 'x' && s[length - 1] != 'Y';
            bool second = length == 2 && is_vowel(s[0]) && !is_vowel(s[1]);

            return first || second;
        }

        void set_initial_y(std::string &s, const int &start) {
            if (s.front() == 'y') s[0] = 'Y';
            auto length = s.length();

            for (int i = start; i < length; ++i) {
                if (s[i] == 'y' && is_vowel_without_y(s[i - 1])) {
                    s[i] = 'Y';
                    ++i;
                }
            }
        }

        bool is_special(std::string &s) {
            const static std::pair<std::string, std::string> special_words[] = {
                    {"gently", "gentl"},
                    {"singly", "singl"},
                    {"early",  "earli"},
                    {"skies",  "sky"},
                    {"dying",  "die"},
                    {"lying",  "lie"},
                    {"tying",  "tie"},
                    {"skis",   "ski"},
                    {"idly",   "idl"},
                    {"ugly",   "ugli"},
                    {"only",   "onli"},
            };
            const static std::string special_words2[] = {
                    "cosmos",
                    "atlas",
                    "andes",
                    "news",
                    "howe",
                    "bias",
                    "sky",
            };

            for (const auto &special_word: special_words) {
                if (s == special_word.first) {
                    s = special_word.second;
                    return true;
                }
            }
            for (const auto &special_word: special_words2) {
                if (s == special_word) {
                    return true;
                }
            }

            return false;
        }
        bool is_special_1a(const std::string &s) {
            const static std::string special_words[] = {
                    "inning",
                    "outing",
                    "canning",
                    "herring",
                    "earring",
                    "proceed",
                    "exceed",
                    "succeed",
            };

            for (const auto &special_word: special_words) {
                if (s == special_word) {
                    return true;
                }
            }

            return false;
        }
        int is_special_r1(const std::string &word) {
            const static std::string special_words[] = {
                    "gener",
                    "commun",
                    "arsen",
            };

            for (const auto &special_word: special_words) {
                if (starts_with(word, special_word)) {
                    return special_word.length();
                }
            }

            return -1;
        }

        bool is_stop(const std::string &s) {
            const static std::string words[] = {
                    "i", "me", "my", "myself", "we", "our", "ours", "ourselves", "you", "your", "yours", "yourself", "yourselves", "he", "him", "his", "himself", "she", "her", "hers", "herself", "it", "its", "itself", "they", "them", "their", "theirs", "themselves", "what", "which", "who", "whom", "this", "that", "these", "those", "am", "is", "are", "was", "were", "be", "been", "being", "have", "has", "had", "having", "do", "does", "did", "doing", "would", "should", "could", "ought", "i'm", "you're", "he's", "she's", "it's", "we're", "they're", "i've", "you've", "we've", "they've", "i'd", "you'd", "he'd", "she'd", "we'd", "they'd", "i'll", "you'll", "he'll", "she'll", "we'll", "they'll", "isn't", "aren't", "wasn't", "weren't", "hasn't", "haven't", "hadn't", "doesn't", "don't", "didn't", "won't", "wouldn't", "shan't", "shouldn't", "can't", "cannot", "couldn't", "mustn't", "let's", "that's", "who's", "what's", "here's", "there's", "when's", "where's", "why's", "how's", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by", "for", "with", "about", "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under", "again", "further", "then", "once", "here", "there", "when", "where", "why", "how", "all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than", "too", "very",
            };
            const static auto words_size = 173;

            for (short i = 0; i < words_size; ++i) {
                if (s == words[i]) {
                    return true;
                }
            }

            return false;
        }

        void step_0(std::string &word) {
            replace_end(word, "'s'") ||
            replace_end(word, "'s") ||
            replace_end(word, "'");
        }

        void step_1a(std::string &word) {
            auto length = word.length();
            if (replace_end(word, "sses", "ss")) return;

            if (length - 3 > 1) {
                if (replace_end(word, "ied", "i") || replace_end(word, "ies", "i")) {
                    return;
                }
            } else {
                if (replace_end(word, "ied", "ie") || replace_end(word, "ies", "ie")) {
                    return;
                }
            }

            if (ends_with(word, "us") || ends_with(word, "ss")) return;
            if_contains_vowel_replace_end(word, length - 2, "s");
        }
        void step_1b(std::string &word, const int &r1) {
            if (replace_end(word, r1, "eedly", "ee") || replace_end(word, r1, "eed", "ee")) {
                return;
            }
            if (find_end(word, "eedly") != std::string::npos || find_end(word, "eed") != std::string::npos) {
                return;
            }

            bool replaced = if_contains_vowel_replace_end(word, "ingly")
                            ||
                            if_contains_vowel_replace_end(word, "edly")
                            ||
                            if_contains_vowel_replace_end(word, "ing")
                            ||
                            if_contains_vowel_replace_end(word, "ed");

            if (replaced) {
                if (ends_with(word, "at") || ends_with(word, "bl") || ends_with(word, "iz")) {
                    word.push_back('e');
                } else if (ends_with_double(word)) {
                    word.pop_back();
                } else if (is_short(word) && word.length() == r1) {
                    word.push_back('e');
                }
            }
        }
        void step_1c(std::string &word) {
            auto length = word.length();
            if (length <= 2) return;

            if (word[length - 1] == 'y' || word[length - 1] == 'Y') {
                if (!is_vowel_without_y(word[length - 2])) {
                    word[length - 1] = 'i';
                }
            }
        }

        void step_2(std::string &word, const int &r1) {
            //faster than std::pair<std::string, std::string>
            const static std::string suffixes[] = {
                    "ization",
                    "ousness",
                    "iveness",
                    "fulness",
                    "ational",
                    "tional",
                    "lessli",
                    "biliti",
                    "alism",
                    "aliti",
                    "entli",
                    "ation",
                    "ousli",
                    "iviti",
                    "fulli",
                    "abli",
                    "anci",
                    "ator",
                    "enci",
                    "izer",
                    "alli",
                    "bli",
            };
            const static std::string suffixes2[]{
                    "ize",
                    "ous",
                    "ive",
                    "ful",
                    "ate",
                    "tion",
                    "less",
                    "ble",
                    "al",
                    "al",
                    "ent",
                    "ate",
                    "ous",
                    "ive",
                    "ful",
                    "able",
                    "ance",
                    "ate",
                    "ence",
                    "ize",
                    "al",
                    "ble",
            };
            const static auto suffixes_size = 22;

             for (short i = 0; i < suffixes_size; ++i) {
                if (replace_end(word, r1, suffixes[i], suffixes2[i])) {
                    return;
                }
            }

            if (replace_end(word, r1 - 1, "logi", "log")) return;

            for (short i = 0; i < suffixes_size; ++i) {
                if (ends_with(word, suffixes[i])) {
                    return;
                }
            }
            
            auto found = find_end(word, "li");

            if (found != std::string::npos && found >= r1 && is_valid_li_ending(word[found - 1])) {
                replace_end(word, "li");
            }
        }

        void step_3(std::string &word, const int &r1, const int &r2) {
            //faster than std::pair<std::string, std::string>
            const static std::string suffixes[] = {
                    "ational",
                    "tional",
                    "alize",
                    "icate",
                    "iciti",
                    "ical",
                    "ness",
                    "ful",
            };
            const static std::string suffixes2[]{
                    "ate",
                    "tion",
                    "al",
                    "ic",
                    "ic",
                    "ic",
                    "",
                    "",
            };
            const static auto suffixes_size = 8;

            for (short i = 0; i < suffixes_size; ++i) {
                if (replace_end(word, r1, suffixes[i], suffixes2[i])) {
                    return;
                }
            }

            replace_end(word, r2, "ative");
        }

        void step_4(std::string &word, const int &r2) {
            const static std::string suffixes[] = {
                    "ement",
                    "ment",
                    "able",
                    "ance",
                    "ence",
                    "ible",
                    "ate",
                    "ant",
                    "ism",
                    "iti",
                    "ous",
                    "ive",
                    "ize",
                    "al",
                    "er",
                    "ic",
            };
            const static auto suffixes_size = 16;

            for (short i = 0; i < suffixes_size; ++i) {
                if (replace_end(word, r2, suffixes[i])) {
                    return;
                }
            }

            if (!ends_with(word, "ement") && !ends_with(word, "ment")) {
                replace_end(word, r2, "ent");
            }

            auto found = find_end(word, "ion");

            if (found != std::string::npos && found >= r2 - 1 && (word[found - 1] == 's' || word[found - 1] == 't')) {
                replace_end(word, "ion");
            }
        }

        void step_5(std::string &word, const int &r1, const int &r2) {
            auto length = word.length();
            auto found = find_end(word, "e");

            if (found != std::string::npos) {
                if (found >= r2) {
                    replace_end(word, "e");
                    return;
                } else if (found >= r1 && !is_short(word.substr(0, length - (length - found)))) {
                    replace_end(word, "e");
                    return;
                }
            }

            auto found2 = find_end(word, "l");

            if (found2 != std::string::npos && found2 >= r2 && (word[found2 - 1] == 'l')) {
                replace_end(word, r2, "l");
            }
        }

        bool stem(std::string &word) {
            auto length = word.length();

            if (length <= 1) {
                return false;
            }
            if (length > 40) {
                return true;
            }
            if (is_stop(word)) {
                return false;
            }
            if (is_special(word)) {
                return true;
            }
            if (word.front() == '\'') {
                word.erase(word.begin());
            }

            set_initial_y(word);

            auto r1 = is_special_r1(word);
            if (r1 == -1) r1 = first_non_vowel(word);

            auto r2 = first_non_vowel(word, r1);

            step_0(word);

            step_1a(word);

            if (is_special_1a(word)) {
                return true;
            }

            step_1b(word, r1);
            step_1c(word);

            step_2(word, r1);

            step_3(word, r1, r2);

            step_4(word, r2);

            step_5(word, r1, r2);

            replace(word, "Y", "y");
            return true;
        }
    }
}