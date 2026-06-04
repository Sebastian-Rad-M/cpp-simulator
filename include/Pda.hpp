#pragma once

#include <string>
#include <vector>

enum class AcceptanceCondition {
    FinalState,
    EmptyStack,
    Both
};

struct PdaTransition {
    int from;
    int to;
    std::string input_symbol; // "" for lambda
    char pop_symbol;
    std::string push_symbols;
};

class Pda {
public:
    int initial_state;
    char initial_stack_symbol;
    std::vector<int> final_states;
    std::vector<PdaTransition> edges;
    AcceptanceCondition acceptance_condition;

    Pda() = default;

    bool run(const std::string& input) const;
};
