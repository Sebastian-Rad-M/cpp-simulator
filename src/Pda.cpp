#include "Pda.hpp"
#include <set>
#include <map>
#include <algorithm>

static std::set<std::pair<int, std::vector<char>>> epsilon_closure(
    const Pda& pda,
    std::set<std::pair<int, std::vector<char>>> configs
) {
    std::vector<std::pair<int, std::vector<char>>> stack(configs.begin(), configs.end());

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        int state = current.first;
        std::vector<char> current_stack = current.second;

        if (current_stack.empty()) continue;

        char top = current_stack.back();

        for (const auto& edge : pda.edges) {
            if (edge.from == state && edge.input_symbol.empty() && edge.pop_symbol == top) {
                std::vector<char> next_stack = current_stack;
                next_stack.pop_back();
                
                for (auto it = edge.push_symbols.rbegin(); it != edge.push_symbols.rend(); it++) {
                    next_stack.push_back(*it);
                }

                std::pair<int, std::vector<char>> next_config = {edge.to, next_stack};
                if (configs.find(next_config) == configs.end()) {
                    configs.insert(next_config);
                    stack.push_back(next_config);
                }
            }
        }
    }
    return configs;
}

bool Pda::run(const std::string& input) const {
    std::set<std::pair<int, std::vector<char>>> current_configs;
    current_configs.insert({initial_state, {initial_stack_symbol}});

    current_configs = epsilon_closure(*this, current_configs);

    for (char c : input) {
        std::string sym = std::string(1, c);
        std::set<std::pair<int, std::vector<char>>> next_configs;

        for (const auto& config : current_configs) {
            int state = config.first;
            const auto& current_stack = config.second;

            if (current_stack.empty()) continue;
            char top = current_stack.back();

            for (const auto& edge : edges) {
                if (edge.from == state && edge.input_symbol == sym && edge.pop_symbol == top) {
                    std::vector<char> next_stack = current_stack;
                    next_stack.pop_back();

                    for (auto it = edge.push_symbols.rbegin(); it != edge.push_symbols.rend(); ++it) {
                        next_stack.push_back(*it);
                    }

                    next_configs.insert({edge.to, next_stack});
                }
            }
        }

        current_configs = epsilon_closure(*this, next_configs);
        if (current_configs.empty()) {
            return false;
        }
    }

    for (const auto& config : current_configs) {
        bool is_final = std::find(final_states.begin(), final_states.end(), config.first) != final_states.end();
        bool is_empty = config.second.empty();

        switch (acceptance_condition) {
            case AcceptanceCondition::FinalState:
                if (is_final) return true;
                break;
            case AcceptanceCondition::EmptyStack:
                if (is_empty) return true;
                break;
            case AcceptanceCondition::Both:
                if (is_final && is_empty) return true;
                break;
        }
    }

    return false;
}
