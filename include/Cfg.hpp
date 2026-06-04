#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

class Cfg {
public:
    std::set<std::string> non_terminals;
    std::set<std::string> terminals;
    std::string start_symbol;
    // head -> list of productions, each production is a list of symbols
    std::map<std::string, std::vector<std::vector<std::string>>> productions;

    Cfg() = default;

    // Read from file
    static Cfg read_from_file(const std::string& path);

    // Ex 1: Generate all words of exact length k
    std::set<std::string> generate_words(size_t k) const;

    // Ex 2: Convert to Chomsky Normal Form
    Cfg to_cnf() const;

    // Ex 3 (variant #2): CYK algorithm
    bool cyk(const std::string& word) const;

    // Debug print
    void print() const;
};
