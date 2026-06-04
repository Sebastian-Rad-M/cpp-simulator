#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

struct Transition {
    int from;
    int to;
    std::string label; // "λ" or "" for lambda, otherwise single char string
};

class LambdaNfa {
public:
    int initial_state;
    std::vector<int> final_states;
    std::vector<Transition> edges;

    LambdaNfa() = default;

    // Convert to Regex (State Elimination Method)
    std::string to_regex() const;

    // Simulated run
    bool run(const std::string& input) const;
};
