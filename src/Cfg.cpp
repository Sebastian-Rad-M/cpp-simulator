#include "Cfg.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <cassert>

// ----- Helpers -----

static std::vector<std::string> split_whitespace(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ----- File I/O -----

Cfg Cfg::read_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    Cfg cfg;
    std::string line;

    // Line 1: Non-terminals
    if (std::getline(file, line)) {
        for (auto& tok : split_whitespace(line)) {
            cfg.non_terminals.insert(tok);
        }
    }

    // Line 2: Terminals
    if (std::getline(file, line)) {
        for (auto& tok : split_whitespace(line)) {
            cfg.terminals.insert(tok);
        }
    }

    // Line 3: Start symbol
    if (std::getline(file, line)) {
        cfg.start_symbol = trim(line);
    }

    // Remaining lines: Productions  A -> alfa | beta
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        auto arrow_pos = line.find("->");
        if (arrow_pos == std::string::npos) continue;

        std::string head = trim(line.substr(0, arrow_pos));
        std::string rhs = line.substr(arrow_pos + 2);

        // Split by '|'
        std::istringstream rhs_stream(rhs);
        std::string body;
        while (std::getline(rhs_stream, body, '|')) {
            auto symbols = split_whitespace(body);
            std::vector<std::string> prod;
            for (auto& sym : symbols) {
                if (sym == "lambda" || sym == "epsilon" ||
                    sym == "λ" || sym == "ε") {
                    // lambda production -> empty vector
                } else {
                    prod.push_back(sym);
                }
            }
            cfg.productions[head].push_back(prod);
        }
    }

    return cfg;
}

// ----- Debug print -----

void Cfg::print() const {
    std::cout << "Non-terminals: ";
    for (auto& nt : non_terminals) std::cout << nt << " ";
    std::cout << "\nTerminals: ";
    for (auto& t : terminals) std::cout << t << " ";
    std::cout << "\nStart: " << start_symbol << "\nProductions:\n";
    for (auto& [head, prods] : productions) {
        for (auto& prod : prods) {
            std::cout << "  " << head << " -> ";
            if (prod.empty()) {
                std::cout << "λ";
            } else {
                for (auto& s : prod) std::cout << s << " ";
            }
            std::cout << "\n";
        }
    }
}

// ----- Ex 1: Generate words of length k -----

std::set<std::string> Cfg::generate_words(size_t k) const {
    std::set<std::string> results;
    std::queue<std::vector<std::string>> queue;
    queue.push({start_symbol});

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        // Count terminals and non-terminals
        size_t terminal_count = 0;
        size_t non_terminal_count = 0;
        for (auto& s : current) {
            if (terminals.count(s)) {
                terminal_count++;
            } else if (non_terminals.count(s)) {
                non_terminal_count++;
            }
        }

        if (non_terminal_count == 0) {
            if (terminal_count == k) {
                std::string word;
                for (auto& s : current) word += s;
                results.insert(word);
            }
            continue;
        }

        // Prune: if terminal count already exceeds k, abandon
        if (terminal_count > k) {
            continue;
        }

        // Prune: if total length > k and no lambda productions exist
        bool has_lambda = false;
        for (auto& [head, prods] : productions) {
            for (auto& prod : prods) {
                if (prod.empty()) { has_lambda = true; break; }
            }
            if (has_lambda) break;
        }
        if (current.size() > k && !has_lambda) {
            continue;
        }

        // Safety limit
        if (current.size() > k + 20) {
            continue;
        }

        // Find first non-terminal and expand
        for (size_t pos = 0; pos < current.size(); pos++) {
            if (non_terminals.count(current[pos])) {
                auto it = productions.find(current[pos]);
                if (it != productions.end()) {
                    for (auto& prod : it->second) {
                        std::vector<std::string> next;
                        // Copy elements before pos
                        next.insert(next.end(), current.begin(), current.begin() + pos);
                        // Insert production
                        next.insert(next.end(), prod.begin(), prod.end());
                        // Copy elements after pos
                        next.insert(next.end(), current.begin() + pos + 1, current.end());
                        queue.push(next);
                    }
                }
                break; // Only expand the first non-terminal
            }
        }
    }

    return results;
}

// ----- Ex 2: Convert to Chomsky Normal Form -----

Cfg Cfg::to_cnf() const {
    Cfg cnf = *this;

    // Step 1: START - Add new start symbol if S appears in RHS
    bool s_in_rhs = false;
    for (auto& [head, prods] : cnf.productions) {
        for (auto& prod : prods) {
            for (auto& sym : prod) {
                if (sym == cnf.start_symbol) {
                    s_in_rhs = true;
                    break;
                }
            }
            if (s_in_rhs) break;
        }
        if (s_in_rhs) break;
    }
    if (s_in_rhs) {
        std::string old_start = cnf.start_symbol;
        cnf.start_symbol = "S0";
        cnf.non_terminals.insert("S0");
        cnf.productions["S0"] = {{old_start}};
    }

    // Step 2: TERM - Replace terminals in mixed rules
    std::map<std::string, std::string> terminal_to_nt;
    std::map<std::string, std::vector<std::vector<std::string>>> new_productions;

    for (auto& [head, prods] : cnf.productions) {
        std::vector<std::vector<std::string>> updated_prods;
        for (auto& prod : prods) {
            if (prod.size() > 1 || (prod.size() == 1 && cnf.non_terminals.count(prod[0]))) {
                std::vector<std::string> new_prod;
                for (auto& sym : prod) {
                    if (cnf.terminals.count(sym)) {
                        if (!terminal_to_nt.count(sym)) {
                            terminal_to_nt[sym] = "X_" + sym;
                        }
                        new_prod.push_back(terminal_to_nt[sym]);
                    } else {
                        new_prod.push_back(sym);
                    }
                }
                updated_prods.push_back(new_prod);
            } else {
                updated_prods.push_back(prod);
            }
        }
        new_productions[head] = updated_prods;
    }

    for (auto& [term, nt] : terminal_to_nt) {
        cnf.non_terminals.insert(nt);
        new_productions[nt] = {{term}};
    }
    cnf.productions = new_productions;

    // Step 3: BIN - Binarize rules with more than 2 symbols on RHS
    std::map<std::string, std::vector<std::vector<std::string>>> binarized_prods;
    int bin_counter = 0;

    for (auto& [head, prods] : cnf.productions) {
        std::vector<std::vector<std::string>> updated_prods;
        for (auto& prod : prods) {
            if (prod.size() > 2) {
                std::string current_head = head;
                for (size_t i = 0; i < prod.size() - 2; i++) {
                    std::string new_nt = "C_" + std::to_string(bin_counter);
                    bin_counter++;
                    cnf.non_terminals.insert(new_nt);

                    binarized_prods[current_head].push_back({prod[i], new_nt});
                    current_head = new_nt;
                }
                binarized_prods[current_head].push_back(
                    {prod[prod.size() - 2], prod[prod.size() - 1]});
            } else {
                updated_prods.push_back(prod);
            }
        }
        if (!updated_prods.empty()) {
            auto& entry = binarized_prods[head];
            entry.insert(entry.end(), updated_prods.begin(), updated_prods.end());
        }
    }
    cnf.productions = binarized_prods;

    // Step 4: DEL - Lambda elimination
    std::set<std::string> nullable;
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [head, prods] : cnf.productions) {
            if (nullable.count(head)) continue;
            for (auto& prod : prods) {
                if (prod.empty() ||
                    std::all_of(prod.begin(), prod.end(),
                                [&](const std::string& s) {
                                    return nullable.count(s) > 0;
                                })) {
                    nullable.insert(head);
                    changed = true;
                    break;
                }
            }
        }
    }

    std::map<std::string, std::vector<std::vector<std::string>>> no_lambda_prods;
    for (auto& [head, prods] : cnf.productions) {
        std::set<std::vector<std::string>> updated_prods;
        for (auto& prod : prods) {
            if (prod.empty()) continue;

            // Generate all subsets where nullable symbols can be removed
            std::vector<std::vector<std::string>> current_gen = {{}};
            for (auto& sym : prod) {
                std::vector<std::vector<std::string>> next_gen;
                for (auto& p : current_gen) {
                    // Option 1: keep sym
                    auto p1 = p;
                    p1.push_back(sym);
                    next_gen.push_back(p1);

                    // Option 2: remove sym (if nullable)
                    if (nullable.count(sym)) {
                        next_gen.push_back(p);
                    }
                }
                current_gen = next_gen;
            }
            for (auto& p : current_gen) {
                if (!p.empty()) {
                    updated_prods.insert(p);
                }
            }
        }
        no_lambda_prods[head] = std::vector<std::vector<std::string>>(
            updated_prods.begin(), updated_prods.end());
    }
    cnf.productions = no_lambda_prods;

    // Step 5: UNIT - Eliminate unit productions
    std::map<std::string, std::set<std::string>> unit_closures;
    for (auto& nt : cnf.non_terminals) {
        std::set<std::string> closure;
        std::vector<std::string> stack = {nt};
        while (!stack.empty()) {
            std::string current = stack.back();
            stack.pop_back();
            auto it = cnf.productions.find(current);
            if (it != cnf.productions.end()) {
                for (auto& prod : it->second) {
                    if (prod.size() == 1 && cnf.non_terminals.count(prod[0])) {
                        if (closure.insert(prod[0]).second) {
                            stack.push_back(prod[0]);
                        }
                    }
                }
            }
        }
        unit_closures[nt] = closure;
    }

    std::map<std::string, std::vector<std::vector<std::string>>> final_prods;
    for (auto& [head, prods] : cnf.productions) {
        std::set<std::vector<std::string>> updated_prods;
        // Non-unit productions
        for (auto& prod : prods) {
            if (prod.size() != 1 || !cnf.non_terminals.count(prod[0])) {
                updated_prods.insert(prod);
            }
        }
        // Productions from unit closure
        if (unit_closures.count(head)) {
            for (auto& unit_nt : unit_closures[head]) {
                auto it = cnf.productions.find(unit_nt);
                if (it != cnf.productions.end()) {
                    for (auto& prod : it->second) {
                        if (prod.size() != 1 || !cnf.non_terminals.count(prod[0])) {
                            updated_prods.insert(prod);
                        }
                    }
                }
            }
        }
        final_prods[head] = std::vector<std::vector<std::string>>(
            updated_prods.begin(), updated_prods.end());
    }
    cnf.productions = final_prods;

    return cnf;
}

// ----- Ex 3 (variant #2): CYK Algorithm -----

bool Cfg::cyk(const std::string& word) const {
    size_t n = word.size();
    if (n == 0) {
        return false;
    }

    // T[i][j] = set of non-terminals that can generate word[i..i+j-1]
    // i = start index (0-based), j = length (1-based)
    std::vector<std::vector<std::set<std::string>>> table(n, std::vector<std::set<std::string>>(n + 1));

    // Step 1: Base case (j = 1)
    for (size_t i = 0; i < n; i++) {
        std::string sym(1, word[i]);
        for (auto& [head, prods] : productions) {
            for (auto& prod : prods) {
                if (prod.size() == 1 && prod[0] == sym) {
                    table[i][1].insert(head);
                }
            }
        }
    }

    // Step 2: Recursive step (j = 2..n)
    for (size_t j = 2; j <= n; j++) {
        for (size_t i = 0; i + j <= n; i++) {
            std::set<std::string> added_heads;
            for (size_t k = 1; k < j; k++) {
                auto& set_b = table[i][k];
                auto& set_c = table[i + k][j - k];

                if (set_b.empty() || set_c.empty()) continue;

                for (auto& [head, prods] : productions) {
                    for (auto& prod : prods) {
                        if (prod.size() == 2) {
                            if (set_b.count(prod[0]) && set_c.count(prod[1])) {
                                added_heads.insert(head);
                            }
                        }
                    }
                }
            }
            table[i][j].insert(added_heads.begin(), added_heads.end());
        }
    }

    return table[0][n].count(start_symbol) > 0;
}
