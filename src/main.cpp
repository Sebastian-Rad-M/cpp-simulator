#include <iostream>
#include "LambdaNfa.hpp"
#include "Pda.hpp"

void test_complex_nfa_to_regex() {
    std::cout << "--- Testing Lambda-NFA -> Regex (State Elimination) ---" << std::endl;
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

int main() {
    test_complex_nfa_to_regex();
    test_pda();
    return 0;
}
