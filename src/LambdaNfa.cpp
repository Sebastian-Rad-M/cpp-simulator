#include "LambdaNfa.hpp"
#include <algorithm>
#include <iostream>

static std::string concat(const std::string& a, const std::string& b) {
    if (a == "λ" || a.empty()) return b;
    if (b == "λ" || b.empty()) return a;
    return "(" + a + ")(" + b + ")";
}

static std::string union_op(const std::string& a, const std::string& b) {
    if (a == b) return a;
    if (a == "∅" || a.empty()) return b;
    if (b == "∅" || b.empty()) return a;
    if (a == "λ" && b == "λ") return "λ";
    return "(" + a + "|" + b + ")";
}

static std::string star(const std::string& a) {
    if (a == "λ" || a.empty() || a == "∅") return "λ";
    return "(" + a + ")*";
}

std::string LambdaNfa::to_regex() const {
    std::set<int> states_to_eliminate;
    std::map<std::pair<int, int>, std::set<std::string>> grouped_transitions;

    for (const auto& edge : edges) {
        states_to_eliminate.insert(edge.from);
        states_to_eliminate.insert(edge.to);
        
        std::string label = (edge.label.empty() || edge.label == "λ") ? "λ" : edge.label;
        grouped_transitions[{edge.from, edge.to}].insert(label);
    }
    states_to_eliminate.insert(initial_state);
    for (int f : final_states) states_to_eliminate.insert(f);

    std::map<std::pair<int, int>, std::string> transitions;
    for (auto const& [key, labels] : grouped_transitions) {
        std::string combined = "";
        for (const auto& l : labels) {
            combined = union_op(combined, l);
        }
        transitions[key] = combined;
    }

    int max_state = -1;
    if (!states_to_eliminate.empty()) {
        max_state = *std::max_element(states_to_eliminate.begin(), states_to_eliminate.end());
    }
    int start_node = max_state + 1;
    int final_node = max_state + 2;

    transitions[{start_node, initial_state}] = "λ";
    for (int f : final_states) {
        transitions[{f, final_node}] = union_op(transitions.count({f, final_node}) ? transitions[{f, final_node}] : "", "λ");
    }

    for (int k : states_to_eliminate) {
        std::string r_kk = transitions.count({k, k}) ? transitions[{k, k}] : "";
        
        std::vector<int> predecessors;
        std::vector<int> successors;

        for (auto const& [key, label] : transitions) {
            if (key.second == k && key.first != k) predecessors.push_back(key.first);
            if (key.first == k && key.second != k) successors.push_back(key.second);
        }

        std::sort(predecessors.begin(), predecessors.end());
        predecessors.erase(std::unique(predecessors.begin(), predecessors.end()), predecessors.end());
        std::sort(successors.begin(), successors.end());
        successors.erase(std::unique(successors.begin(), successors.end()), successors.end());

        for (int p : predecessors) {
            for (int r : successors) {
                std::string r_pk = transitions[{p, k}];
                std::string r_kr = transitions[{k, r}];

                std::string path_through_k = concat(r_pk, concat(star(r_kk), r_kr));
                
                std::string r_pr = transitions.count({p, r}) ? transitions[{p, r}] : "";
                transitions[{p, r}] = union_op(r_pr, path_through_k);
            }
        }

        for (auto it = transitions.begin(); it != transitions.end(); ) {
            if (it->first.first == k || it->first.second == k) {
                it = transitions.erase(it);
            } else {
                ++it;
            }
        }
    }

    if (transitions.count({start_node, final_node})) {
        return transitions[{start_node, final_node}];
    }
    return "∅";
}

static std::set<int> advance_empty_word(const std::vector<Transition>& edges, const std::set<int>& states) {
    std::set<int> result = states;
    std::vector<int> stack(states.begin(), states.end());

    while (!stack.empty()) {
        int s = stack.back();
        stack.pop_back();

        for (const auto& edge : edges) {
            if (edge.from == s && (edge.label.empty() || edge.label == "λ")) {
                if (result.find(edge.to) == result.end()) {
                    result.insert(edge.to);
                    stack.push_back(edge.to);
                }
            }
        }
    }
    return result;
}

bool LambdaNfa::run(const std::string& input) const {
    std::set<int> current_states = {initial_state};
    current_states = advance_empty_word(edges, current_states);

    for (char c : input) {
        std::string word(1, c);
        std::set<int> next_states;
        for (int state : current_states) {
            for (const auto& edge : edges) {
                if (edge.from == state && edge.label == word) {
                    next_states.insert(edge.to);
                }
            }
        }
        current_states = advance_empty_word(edges, next_states);
        if (current_states.empty()) return false;
    }

    for (int state : current_states) {
        if (std::find(final_states.begin(), final_states.end(), state) != final_states.end()) {
            return true;
        }
    }
    return false;
}
