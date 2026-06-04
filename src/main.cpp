#include "Cfg.hpp"
#include "LambdaNfa.hpp"
#include "Pda.hpp"
#include <algorithm>
#include <iostream>

void test_complex_nfa_to_regex() {
  std::cout << "--- Testing Lambda-NFA -> Regex (State Elimination) ---"
            << std::endl;
  LambdaNfa nfa;
  nfa.initial_state = 0;
  nfa.final_states = {1, 2};

  nfa.edges.push_back({0, 0, "a"});
  nfa.edges.push_back({0, 1, "c"});
  nfa.edges.push_back({1, 1, "b"});

  nfa.edges.push_back({0, 2, "b"});
  nfa.edges.push_back({2, 2, "c"});

  std::string regex_back = nfa.to_regex();
  std::cout << "Generated Regex: " << regex_back << std::endl;
  std::cout << std::endl;
}
void test_pda() {
  std::cout << "\nPDA" << std::endl;
  Pda pda;
  pda.initial_state = 0;
  pda.initial_stack_symbol = 'Z';
  pda.final_states = {2};
  pda.acceptance_condition = AcceptanceCondition::Both;
  pda.edges.push_back({0, 0, "a", 'Z', "AZ"});
  pda.edges.push_back({0, 0, "b", 'Z', "BZ"});
  pda.edges.push_back({0, 0, "a", 'A', "AA"});
  pda.edges.push_back({0, 0, "b", 'B', "BB"});
  pda.edges.push_back({0, 0, "a", 'B', "AB"});
  pda.edges.push_back({0, 0, "b", 'A', "BA"});
  pda.edges.push_back({0, 1, "a", 'A', ""});
  pda.edges.push_back({0, 1, "b", 'B', ""});
  pda.edges.push_back({1, 1, "a", 'A', ""});
  pda.edges.push_back({1, 1, "b", 'B', ""});
  pda.edges.push_back({1, 2, "", 'Z', ""});
  pda.edges.push_back({0, 2, "", 'Z', ""});

  std::cout << "'': " << (pda.run("") ? "Yes" : "No") << std::endl;
  std::cout << "'aa': " << (pda.run("aa") ? "Yes" : "No") << std::endl;
  std::cout << "'abba': " << (pda.run("abba") ? "Yes" : "No") << std::endl;
  std::cout << "'ababa': " << (pda.run("ababa") ? "Yes" : "No") << std::endl;
  std::cout << "'abba': " << (pda.run("abba") ? "Yes" : "No") << std::endl;
  std::cout << "'baab': " << (pda.run("baab") ? "Yes" : "No") << std::endl;
  std::cout << "'ab': " << (pda.run("ab") ? "Yes" : "No") << std::endl;
  std::cout << "'aba': " << (pda.run("aba") ? "Yes" : "No") << std::endl;
  std::cout << "'aaaa': " << (pda.run("aaaa") ? "Yes" : "No") << std::endl;
  std::cout << "'bbbb': " << (pda.run("aaaa") ? "Yes" : "No") << std::endl;
  std::cout << "'ababbaba': " << (pda.run("aaaa") ? "Yes" : "No") << std::endl;
}

void test_cfg_from_file(const std::string &path) {
  std::cout << "\n--- CFG from file: " << path << " ---" << std::endl;
  try {
    Cfg cfg = Cfg::read_from_file(path);
    cfg.print();

    // Ex 1: Generate words of length 4
    std::cout << "\nEx 1: Generate words of length 4" << std::endl;
    auto words = cfg.generate_words(4);
    std::cout << "Words of length 4:";
    for (auto &w : words)
      std::cout << " \"" << w << "\"";
    std::cout << std::endl;

    // Ex 2: Transform to CNF
    std::cout << "\nEx 2: Transform to CNF" << std::endl;
    Cfg cnf = cfg.to_cnf();
    cnf.print();

    // Ex 3: CYK Algorithm
    std::cout << "\nEx 3: CYK Algorithm" << std::endl;
    std::vector<std::string> test_words = {"aabb", "abab",   "ab", "ba",
                                           "baba", "aaabbb", "aab"};
    for (auto &word : test_words) {
      std::cout << "  \"" << word << "\": " << (cnf.cyk(word) ? "DA" : "NU")
                << std::endl;
    }
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}

void test_cfg_hardcoded() {
  std::cout << "\n--- CFG (hardcoded: S -> a S b | a b) ---" << std::endl;
  Cfg cfg;
  cfg.non_terminals = {"S"};
  cfg.terminals = {"a", "b"};
  cfg.start_symbol = "S";
  cfg.productions["S"] = {{"a", "S", "b"}, {"a", "b"}};
  cfg.print();

  // Ex 1: Generate words
  for (size_t k = 1; k <= 8; k++) {
    auto words = cfg.generate_words(k);
    if (!words.empty()) {
      std::cout << "Words of length " << k << ":";
      for (auto &w : words)
        std::cout << " \"" << w << "\"";
      std::cout << std::endl;
    }
  }

  // Ex 2: CNF
  std::cout << "\nCNF:" << std::endl;
  Cfg cnf = cfg.to_cnf();
  cnf.print();

  // Ex 3: CYK
  std::cout << "\nCYK:" << std::endl;
  std::vector<std::string> test_words = {"ab", "aabb", "aaabbb", "a",
                                         "b",  "aba",  "ba",     "aab"};
  for (auto &word : test_words) {
    std::cout << "  \"" << word << "\": " << (cnf.cyk(word) ? "DA" : "NU")
              << std::endl;
  }
}

void test_cfg_with_lambda() {
  std::cout << "\n--- CFG (with lambda: S -> a S b | lambda) ---" << std::endl;
  Cfg cfg;
  cfg.non_terminals = {"S"};
  cfg.terminals = {"a", "b"};
  cfg.start_symbol = "S";
  cfg.productions["S"] = {
      {"a", "S", "b"}, {} // lambda production
  };

  // Ex 1
  for (size_t k = 0; k <= 8; k++) {
    auto words = cfg.generate_words(k);
    if (!words.empty()) {
      std::cout << "Words of length " << k << ":";
      for (auto &w : words)
        std::cout << " \"" << w << "\"";
      std::cout << std::endl;
    }
  }

  // Ex 2 + 3
  Cfg cnf = cfg.to_cnf();
  std::cout << "\nCYK:" << std::endl;
  for (auto &word : {"ab", "aabb", "aaabbb", "a", "b", "aba"}) {
    std::cout << "  \"" << word << "\": " << (cnf.cyk(word) ? "DA" : "NU")
              << std::endl;
  }
}

int main(int argc, char *argv[]) {
  test_complex_nfa_to_regex();
  test_pda();

  // debug
  // if (argc > 1) {
  //     test_cfg_from_file(argv[1]);
  //     return 0;
  // }

  test_cfg_hardcoded();
  test_cfg_with_lambda();
  return 0;
}
