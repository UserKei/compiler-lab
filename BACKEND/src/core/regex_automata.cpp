// Author: User_Kei
// update: 2025 06 05
#include "regex_automata.h"
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <stack>
#include <queue>
#include <algorithm>
#include <fstream>
#include <string>
#include <numeric>
#include <sstream>

// 常量和全局类型定义
namespace Constants {
    const int N = 135;
    
    // 字符映射表
    const std::vector<int> CHARACTER_INDEX = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
    };
}

// 数据结构定义
namespace DataStructures {
    class NFAState {
    public:
        std::vector<int> edges[Constants::N];
        bool accept_state = false;
    };

    class DFAState {
    public:
        std::vector<int> edges[Constants::N];
        bool accept_state = false;
    };
    
    // 全局变量
    std::vector<NFAState> nfa_states;
    std::vector<DFAState> dfa_states;
    std::stack<int> state_stack;
    std::set<char> alphabet;
}

// 输入验证模块
namespace InputValidator {
    void validate_input(const std::string &str) {
        // 收集字母表
        for (const auto &c : str) {
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                DataStructures::alphabet.insert(c);
            }
            
            // 检查合法字符
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  c == '(' || c == ')' || c == '*' || c == '|')) {
                std::cout << "非法输入\n表示标识符的正规式 （标识符： 以字母开头， 由字母和数字构成的字符串）\n";
                return;
            }
        }
    }
}

// 正则表达式预处理模块
namespace RegexPreprocessor {
    std::string add_concatenation(const std::string &regexp) {
        std::string result;
        char current, next;
        
        for (auto it = regexp.begin(); it != regexp.end(); ++it) {
            current = *it;
            if (it + 1 != regexp.end()) {
                next = *(it + 1);
                result.push_back(current);
                if (current != '(' && next != ')' && current != '|' && 
                    next != '|' && next != '*') {
                    result.push_back('.');
                }
            }
        }
        result.push_back(regexp.back());
        return result;
    }
}

// 正则表达式转换模块
namespace RegexConverter {
    int get_operator_priority(const char &c) {
        switch (c) {
            case '*': return 3;
            case '.': return 2;
            case '|': return 1;
            default: return 0;
        }
    }
    
    std::string infix_to_postfix(const std::string &regexp) {
        std::string result;
        std::stack<char> operators;
        
        for (const auto &c : regexp) {
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                result.push_back(c);
            }
            else if (c == '(') {
                operators.push(c);
            }
            else if (c == ')') {
                while (!operators.empty() && operators.top() != '(') {
                    result.push_back(operators.top());
                    operators.pop();
                }
                if (!operators.empty()) operators.pop(); // 弹出 '('
            }
            else {
                while (!operators.empty() && operators.top() != '(' && 
                       get_operator_priority(operators.top()) >= get_operator_priority(c)) {
                    result.push_back(operators.top());
                    operators.pop();
                }
                operators.push(c);
            }
        }
        
        while (!operators.empty()) {
            result.push_back(operators.top());
            operators.pop();
        }
        
        return result;
    }
}

// NFA构建模块
namespace NFAConstructor {
    int state_index = 0;
    
    void create_character_nfa(const char &c) {
        DataStructures::nfa_states.push_back(DataStructures::NFAState());
        DataStructures::nfa_states.push_back(DataStructures::NFAState());
        
        DataStructures::nfa_states[state_index].edges[c].push_back(state_index + 1);
        DataStructures::state_stack.push(state_index++);
        DataStructures::state_stack.push(state_index++);
    }
    
    void create_or_nfa() {
        DataStructures::nfa_states.push_back(DataStructures::NFAState());
        DataStructures::nfa_states.push_back(DataStructures::NFAState());

        int d = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int c = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int b = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int a = DataStructures::state_stack.top(); DataStructures::state_stack.pop();

        DataStructures::nfa_states[state_index].edges['#'].push_back(a);
        DataStructures::nfa_states[state_index].edges['#'].push_back(c);
        DataStructures::state_stack.push(state_index++);
        
        DataStructures::nfa_states[b].edges['#'].push_back(state_index);
        DataStructures::nfa_states[d].edges['#'].push_back(state_index);
        DataStructures::state_stack.push(state_index++);
    }
    
    void create_concatenation_nfa() {
        int d = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int c = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int b = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int a = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        
        DataStructures::nfa_states[b].edges['#'].push_back(c);
        DataStructures::state_stack.push(a);
        DataStructures::state_stack.push(d);
    }
    
    void create_kleene_star_nfa() {
        int b = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        int a = DataStructures::state_stack.top(); DataStructures::state_stack.pop();
        
        DataStructures::nfa_states.push_back(DataStructures::NFAState());
        DataStructures::nfa_states.push_back(DataStructures::NFAState());
        
        DataStructures::nfa_states[b].edges['#'].push_back(a);
        DataStructures::nfa_states[state_index].edges['#'].push_back(a);
        DataStructures::nfa_states[state_index].edges['#'].push_back(state_index + 1);
        DataStructures::nfa_states[b].edges['#'].push_back(state_index + 1);
        
        DataStructures::state_stack.push(state_index++);
        DataStructures::state_stack.push(state_index++);
    }
    
    void build_nfa_from_postfix(const std::string &postfix) {
        for (const auto &c : postfix) {
            switch (c) {
                case '*':
                    create_kleene_star_nfa();
                    break;
                case '.':
                    create_concatenation_nfa();
                    break;
                case '|':
                    create_or_nfa();
                    break;
                default:
                    create_character_nfa(c);
                    break;
            }
        }
    }
}

// NFA管理模块
namespace NFAManager {
    std::vector<int> states;
    std::set<int> non_initial_states;
    int initial_state;
    std::set<int> accept_states;
    std::vector<std::tuple<int, char, int>> transitions;
    
    void initialize() {
        states.resize(DataStructures::nfa_states.size());
        std::iota(states.begin(), states.end(), 0);

        for (int i = 0; i < DataStructures::nfa_states.size(); i++) {
            auto &[edges, accept_state] = DataStructures::nfa_states[i];
            
            // 处理字符转移
            for (auto index : Constants::CHARACTER_INDEX) {
                if (!edges[index].empty()) {
                    for (auto j : edges[index]) {
                        transitions.push_back({i, (char)index, j});
                        non_initial_states.insert(j);
                    }
                }
            }
            
            // 处理epsilon转移
            if (!edges['#'].empty()) {
                for (auto j : edges['#']) {
                    transitions.push_back({i, '#', j});
                    non_initial_states.insert(j);
                }
            }
        }

        // 找到初始状态
        for (int i = 0; i < DataStructures::nfa_states.size(); i++) {
            if (non_initial_states.find(i) == non_initial_states.end()) {
                initial_state = i;
                break;
            }
        }

        // 找到接受状态
        for (int i = 0; i < DataStructures::nfa_states.size(); i++) {
            const auto &[edges, accept_state] = DataStructures::nfa_states[i];
            if (accept_state) {
                accept_states.insert(i);
            }
        }
    }
    
    void print_nfa() {
        std::cout << "=== NFA ===\n";
        std::cout << "States: ";
        for (const auto &i : states) {
            std::cout << i << ' ';
        }
        std::cout << '\n';

        std::cout << "Alphabet: ";
        for (const auto &i : DataStructures::alphabet) {
            std::cout << i << ' ';
        }
        std::cout << '\n';

        std::cout << "Initial state: " << initial_state << '\n';

        std::cout << "Transitions: ";
        for (const auto &[from, symbol, to] : transitions) {
            std::cout << "f(" << from << ", " << symbol << ") = " << to << " ";
        }
        std::cout << '\n';

        std::cout << "Accept states: ";
        for (const auto &i : accept_states) {
            std::cout << i << ' ';
        }
        std::cout << '\n';
    }
}

// DFA转换模块
namespace DFAConverter {
    void compute_epsilon_closure(int state, std::set<int> &closure) {
        const auto &[edges, accept_state] = DataStructures::nfa_states[state];
        for (auto next_state : edges['#']) {
            if (closure.find(next_state) == closure.end()) {
                closure.insert(next_state);
                compute_epsilon_closure(next_state, closure);
            }
        }
    }
    
    void print_epsilon_closures(int n, std::set<int>* eps) {
        std::cout << "=== Epsilon Closures ===\n";
        for (int i = 0; i < n; i++) {
            std::cout << i << ": ";
            for (const auto &j : eps[i]) {
                std::cout << j << ' ';
            }
            std::cout << '\n';
        }
        std::cout << "========================\n";
    }
    
    void convert_nfa_to_dfa(int start_state, int nfa_size) {
        DataStructures::dfa_states.clear();
        std::map<std::set<int>, int> state_map;
        std::map<std::set<int>, bool> visited;
        
        // 计算每个状态的epsilon闭包
        std::vector<std::set<int>> epsilon_closures(nfa_size);
        for (int i = 0; i < nfa_size; i++) {
            epsilon_closures[i].insert(i); // 自己也在epsilon闭包中
            compute_epsilon_closure(i, epsilon_closures[i]);
        }
        
        std::set<int> initial_set = epsilon_closures[start_state];
        state_map[initial_set] = 0;
        DataStructures::dfa_states.push_back(DataStructures::DFAState());
        
        print_epsilon_closures(nfa_size, epsilon_closures.data());
        
        std::queue<std::set<int>> queue;
        queue.push(initial_set);
        int dfa_state_count = 0;
        
        while (!queue.empty()) {
            auto current_set = queue.front();
            queue.pop();
            
            if (visited[current_set]) continue;
            visited[current_set] = true;
            
            for (const auto &symbol : DataStructures::alphabet) {
                std::set<int> next_set;
                
                // 对当前集合中的每个状态，找到通过symbol能到达的状态
                for (const auto &state : current_set) {
                    for (auto next_state : DataStructures::nfa_states[state].edges[symbol]) {
                        // 添加到达状态及其epsilon闭包
                        for (const auto &closure_state : epsilon_closures[next_state]) {
                            next_set.insert(closure_state);
                        }
                    }
                }
                
                if (!next_set.empty()) {
                    if (state_map.find(next_set) == state_map.end()) {
                        state_map[next_set] = ++dfa_state_count;
                        DataStructures::dfa_states.push_back(DataStructures::DFAState());
                    }
                    
                    DataStructures::dfa_states[state_map[current_set]].edges[symbol].push_back(state_map[next_set]);
                    
                    // 检查是否为接受状态
                    for (const auto &state : next_set) {
                        if (NFAManager::accept_states.find(state) != NFAManager::accept_states.end()) {
                            DataStructures::dfa_states[state_map[next_set]].accept_state = true;
                            break;
                        }
                    }
                    
                    if (!visited[next_set]) {
                        queue.push(next_set);
                    }
                }
            }
        }
        
        // 检查初始状态是否为接受状态
        for (const auto &state : initial_set) {
            if (NFAManager::accept_states.find(state) != NFAManager::accept_states.end()) {
                DataStructures::dfa_states[0].accept_state = true;
                break;
            }
        }
    }
}

// DFA管理模块
namespace DFAManager {
    int state_count = 0;
    std::vector<int> states;
    int initial_state = 0;
    std::vector<int> accept_states;
    std::vector<std::tuple<int, char, int>> transitions;
    
    void initialize() {
        states.resize(DataStructures::dfa_states.size());
        std::iota(states.begin(), states.end(), 0);
        state_count = DataStructures::dfa_states.size();

        // 构建转移关系
        for (int i = 0; i < state_count; i++) {
            const auto &[edges, accept_state] = DataStructures::dfa_states[i];
            for (auto index : Constants::CHARACTER_INDEX) {
                if (!edges[index].empty()) {
                    for (auto j : edges[index]) {
                        transitions.push_back({i, (char)index, j});
                    }
                }
            }
        }

        // 找到接受状态
        for (int i = 0; i < state_count; i++) {
            if (DataStructures::dfa_states[i].accept_state) {
                accept_states.push_back(i);
            }
        }
    }
    
    void print_dfa() {
        std::cout << "\n=== DFA ===\n";
        std::cout << "States: ";
        for (const auto &i : states) {
            std::cout << i << ' ';
        }
        std::cout << '\n';

        std::cout << "Initial state: " << initial_state << '\n';

        std::cout << "Transitions: ";
        for (const auto &[from, symbol, to] : transitions) {
            std::cout << "f(" << from << ", " << symbol << ") = " << to << " ";
        }
        std::cout << '\n';

        std::cout << "Accept states: ";
        for (const auto &i : accept_states) {
            std::cout << i << ' ';
        }
        std::cout << '\n';
    }
}

// DFA最小化模块
namespace DFAMinimizer {
    std::vector<int> state_partition;
    std::vector<std::set<int>> partitions;
    std::set<std::tuple<int, char, int>> minimized_transitions;
    
    bool can_distinguish(const std::set<int> &partition, char symbol) {
        if (partition.size() <= 1) return false;
        
        std::map<int, std::set<int>> destination_groups;
        
        for (auto state : partition) {
            const auto &[edges, _] = DataStructures::dfa_states[state];
            
            int destination_partition = -1; // 表示没有转移
            if (!edges[symbol].empty()) {
                // 假设每个状态对每个符号只有一个转移
                int next_state = edges[symbol][0];
                destination_partition = state_partition[next_state];
            }
            
            destination_groups[destination_partition].insert(state);
        }
        
        return destination_groups.size() > 1;
    }
    
    std::vector<std::set<int>> split_partition(const std::set<int> &partition, char symbol) {
        std::map<int, std::set<int>> destination_groups;
        
        for (auto state : partition) {
            const auto &[edges, _] = DataStructures::dfa_states[state];
            
            int destination_partition = -1;
            if (!edges[symbol].empty()) {
                int next_state = edges[symbol][0];
                destination_partition = state_partition[next_state];
            }
            
            destination_groups[destination_partition].insert(state);
        }
        
        std::vector<std::set<int>> result;
        for (auto &[_, group] : destination_groups) {
            result.push_back(group);
        }
        
        return result;
    }
    
    void minimize_dfa() {
        state_partition.resize(DFAManager::state_count);
        
        // 初始分区：接受状态和非接受状态
        std::set<int> accepting_states, non_accepting_states;
        for (int i = 0; i < DFAManager::state_count; i++) {
            if (DataStructures::dfa_states[i].accept_state) {
                accepting_states.insert(i);
                state_partition[i] = 1;
            } else {
                non_accepting_states.insert(i);
                state_partition[i] = 0;
            }
        }
        
        partitions.clear();
        if (!non_accepting_states.empty()) partitions.push_back(non_accepting_states);
        if (!accepting_states.empty()) partitions.push_back(accepting_states);
        
        bool changed = true;
        while (changed) {
            changed = false;
            std::vector<std::set<int>> new_partitions;
            
            for (const auto &partition : partitions) {
                bool partition_split = false;
                
                for (auto symbol : DataStructures::alphabet) {
                    if (can_distinguish(partition, symbol)) {
                        auto split_result = split_partition(partition, symbol);
                        for (const auto &new_partition : split_result) {
                            new_partitions.push_back(new_partition);
                        }
                        partition_split = true;
                        changed = true;
                        break;
                    }
                }
                
                if (!partition_split) {
                    new_partitions.push_back(partition);
                }
            }
            
            partitions = new_partitions;
            
            // 更新状态分区编号
            for (int i = 0; i < partitions.size(); i++) {
                for (auto state : partitions[i]) {
                    state_partition[state] = i;
                }
            }
        }
        
        // 构建最小化后的转移关系
        minimized_transitions.clear();
        for (int i = 0; i < DFAManager::state_count; i++) {
            const auto &[edges, _] = DataStructures::dfa_states[i];
            for (const auto &index : Constants::CHARACTER_INDEX) {
                if (!edges[index].empty()) {
                    for (const auto &next_state : edges[index]) {
                        minimized_transitions.insert({
                            state_partition[i], 
                            (char)index, 
                            state_partition[next_state]
                        });
                    }
                }
            }
        }
    }
    
    void print_minimized_dfa() {
        std::cout << "\n=== Minimized DFA ===\n";
        std::cout << "Partitions: " << partitions.size() << "\n";
        for (int i = 0; i < partitions.size(); i++) {
            std::cout << "Partition " << i << ": {";
            bool first = true;
            for (auto state : partitions[i]) {
                if (!first) std::cout << ", ";
                std::cout << state;
                first = false;
            }
            std::cout << "}\n";
        }
        
        std::cout << "Minimized Transitions: ";
        for (const auto& [from, symbol, to] : minimized_transitions) {
            std::cout << "f(" << from << ", " << symbol << ") = " << to << " ";
        }
        std::cout << "\n";
    }
    
    void export_to_dot(const std::string &filename) {
        std::ofstream out(filename);
        out << "digraph DFA {\n";
        out << "  rankdir=LR;\n";

        // 确定接受状态
        std::set<int> accept_partitions;
        for (const auto &state : DFAManager::accept_states) {
            accept_partitions.insert(state_partition[state]);
        }
        
        // 输出非接受状态
        out << "  node [shape = circle];\n";
        for (int i = 0; i < partitions.size(); ++i) {
            if (accept_partitions.find(i) == accept_partitions.end()) {
                out << "  " << i << ";\n";
            }
        }
        
        // 输出接受状态
        if (!accept_partitions.empty()) {
            out << "  node [shape = doublecircle];\n";
            for (int state : accept_partitions) {
                out << "  " << state << ";\n";
            }
        }
        
        // 初始状态箭头
        int initial_partition = state_partition[0];
        out << "  __start [shape=point];\n";
        out << "  __start -> " << initial_partition << ";\n";
        
        // 输出转移关系
        for (const auto &[src, symbol, dest] : minimized_transitions) {
            out << "  " << src << " -> " << dest << " [label=\"" << symbol << "\"];\n";
        }
        
        out << "}\n";
        out.close();
    }
}

// API接口命名空间 - 用于web后端集成
namespace RegexBuilder {
    // 全局状态变量
    static bool isInitialized = false;
    static std::string currentRegex;
    static std::string nfaDescription;
    static std::string dfaDescription;
    
    // 重置所有状态
    void resetState() {
        DataStructures::nfa_states.clear();
        DataStructures::dfa_states.clear();
        while (!DataStructures::state_stack.empty()) {
            DataStructures::state_stack.pop();
        }
        DataStructures::alphabet.clear();
        NFAConstructor::state_index = 0;
        NFAManager::states.clear();
        NFAManager::non_initial_states.clear();
        NFAManager::accept_states.clear();
        NFAManager::transitions.clear();
        DFAManager::states.clear();
        DFAManager::accept_states.clear();
        DFAManager::transitions.clear();
        DFAMinimizer::state_partition.clear();
        DFAMinimizer::partitions.clear();
        DFAMinimizer::minimized_transitions.clear();
        isInitialized = false;
        currentRegex.clear();
        nfaDescription.clear();
        dfaDescription.clear();
    }
    
    // 生成NFA和DFA的描述信息
    void generateDescriptions();
    
    // 从正则表达式构建自动机
    bool buildAutomataFromRegex(const std::string& regex) {
        try {
            resetState();
            currentRegex = regex;
            
            // 1. 输入验证
            InputValidator::validate_input(regex);
            
            // 2. 预处理正则表达式
            std::string processed_regex = RegexPreprocessor::add_concatenation(regex);
            
            // 3. 转换为后缀表达式
            std::string postfix = RegexConverter::infix_to_postfix(processed_regex);
            
            // 4. 构建NFA
            NFAConstructor::build_nfa_from_postfix(postfix);
            
            // 5. 设置最终状态
            if (DataStructures::state_stack.size() >= 2) {
                int final_state = DataStructures::state_stack.top(); 
                DataStructures::state_stack.pop();
                int start_state = DataStructures::state_stack.top(); 
                DataStructures::state_stack.pop();
                
                DataStructures::nfa_states[final_state].accept_state = true;
                
                // 初始化NFA管理器（现在接受状态已经设置好了）
                NFAManager::initialize();
                
                // 6. 转换为DFA
                DFAConverter::convert_nfa_to_dfa(start_state, DataStructures::nfa_states.size());
                DFAManager::initialize();
                
                // 7. 最小化DFA
                DFAMinimizer::minimize_dfa();
                
                // 8. 生成描述信息
                generateDescriptions();
                
                isInitialized = true;
                return true;
            }
            
            return false;
            
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    // 匹配字符串
    bool matchString(const std::string& input) {
        if (!isInitialized) {
            return false;
        }
        
        try {
            int current_state = DFAMinimizer::state_partition[0]; // 初始状态的分区
            
            for (char c : input) {
                bool found_transition = false;
                
                // 在最小化的转移关系中查找
                for (const auto& [from, symbol, to] : DFAMinimizer::minimized_transitions) {
                    if (from == current_state && symbol == c) {
                        current_state = to;
                        found_transition = true;
                        break;
                    }
                }
                
                if (!found_transition) {
                    return false; // 没有找到转移，拒绝
                }
            }
            
            // 检查最终状态是否为接受状态
            for (const auto& accept_state : DFAManager::accept_states) {
                if (DFAMinimizer::state_partition[accept_state] == current_state) {
                    return true;
                }
            }
            
            return false;
            
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    // 生成NFA和DFA的描述信息
    void generateDescriptions() {
        // 生成NFA描述
        std::ostringstream nfa_oss;
        nfa_oss << "NFA States: " << NFAManager::states.size() << "\n";
        nfa_oss << "Initial State: " << NFAManager::initial_state << "\n";
        nfa_oss << "Accept States: ";
        for (const auto& state : NFAManager::accept_states) {
            nfa_oss << state << " ";
        }
        nfa_oss << "\nTransitions: " << NFAManager::transitions.size() << " total\n";
        for (const auto& [from, symbol, to] : NFAManager::transitions) {
            nfa_oss << "  δ(" << from << ", " << symbol << ") = " << to << "\n";
        }
        nfaDescription = nfa_oss.str();
        
        // 生成DFA描述
        std::ostringstream dfa_oss;
        dfa_oss << "DFA States: " << DFAManager::states.size() << "\n";
        dfa_oss << "Initial State: " << DFAManager::initial_state << "\n";
        dfa_oss << "Accept States: ";
        for (const auto& state : DFAManager::accept_states) {
            dfa_oss << state << " ";
        }
        dfa_oss << "\nTransitions: " << DFAManager::transitions.size() << " total\n";
        for (const auto& [from, symbol, to] : DFAManager::transitions) {
            dfa_oss << "  δ(" << from << ", " << symbol << ") = " << to << "\n";
        }
        
        // 添加最小化信息
        dfa_oss << "\nMinimized DFA:\n";
        dfa_oss << "Partitions: " << DFAMinimizer::partitions.size() << "\n";
        for (int i = 0; i < DFAMinimizer::partitions.size(); i++) {
            dfa_oss << "  Partition " << i << ": {";
            bool first = true;
            for (auto state : DFAMinimizer::partitions[i]) {
                if (!first) dfa_oss << ", ";
                dfa_oss << state;
                first = false;
            }
            dfa_oss << "}\n";
        }
        dfa_oss << "Minimized Transitions: " << DFAMinimizer::minimized_transitions.size() << " total\n";
        for (const auto& [from, symbol, to] : DFAMinimizer::minimized_transitions) {
            dfa_oss << "  δ(" << from << ", " << symbol << ") = " << to << "\n";
        }
        
        dfaDescription = dfa_oss.str();
    }
    
    // 获取NFA描述
    std::string getNFADescription() {
        return nfaDescription;
    }
    
    // 获取DFA描述
    std::string getDFADescription() {
        return dfaDescription;
    }
    
    // 获取当前正则表达式
    std::string getCurrentRegex() {
        return currentRegex;
    }
    
    // 检查是否已初始化
    bool isReady() {
        return isInitialized;
    }
    
    // 生成NFA的DOT文件内容
    std::string generateNFADotContent() {
        if (!isInitialized) return "";
        
        std::stringstream dot;
        dot << "digraph NFA {" << std::endl;
        dot << "    rankdir=LR;" << std::endl;
        dot << "    node [shape=circle];" << std::endl;
        dot << std::endl;
        
        // 创建一个隐藏的起始节点
        dot << "    __start [shape=point];" << std::endl;
        dot << "    __start -> " << NFAManager::initial_state << ";" << std::endl;
        dot << std::endl;
        
        // 标记接受状态
        if (!NFAManager::accept_states.empty()) {
            dot << "    node [shape=doublecircle];" << std::endl;
            for (const auto& state : NFAManager::accept_states) {
                dot << "    " << state << ";" << std::endl;
            }
            dot << "    node [shape=circle];" << std::endl;
        }
        dot << std::endl;
        
        // 生成转移边
        for (const auto& [from, symbol, to] : NFAManager::transitions) {
            dot << "    " << from << " -> " << to;
            if (symbol == '#') {
                dot << " [label=\"ε\"];" << std::endl;  // epsilon转换
            } else {
                dot << " [label=\"" << symbol << "\"];" << std::endl;
            }
        }
        
        dot << "}" << std::endl;
        return dot.str();
    }
    
    // 生成DFA的DOT文件内容
    std::string generateDFADotContent() {
        if (!isInitialized) return "";
        
        std::stringstream dot;
        dot << "digraph DFA {" << std::endl;
        dot << "    rankdir=LR;" << std::endl;
        dot << std::endl;
        
        // 创建一个隐藏的起始节点
        dot << "    __start [shape=point];" << std::endl;
        dot << "    __start -> " << DFAManager::initial_state << ";" << std::endl;
        dot << std::endl;
        
        // 确定接受状态集合
        std::set<int> accept_states_set(DFAManager::accept_states.begin(), DFAManager::accept_states.end());
        
        // 为每个状态设置正确的形状
        for (int i = 0; i < DFAManager::state_count; i++) {
            if (accept_states_set.find(i) != accept_states_set.end()) {
                dot << "    " << i << " [shape=doublecircle];" << std::endl;
            } else {
                dot << "    " << i << " [shape=circle];" << std::endl;
            }
        }
        dot << std::endl;
        
        // 生成转移边
        for (const auto& [from, symbol, to] : DFAManager::transitions) {
            dot << "    " << from << " -> " << to;
            dot << " [label=\"" << symbol << "\"];" << std::endl;
        }
        
        dot << "}" << std::endl;
        return dot.str();
    }
    
    // 生成最小化DFA的DOT文件内容
    std::string generateMinimizedDFADotContent() {
        if (!isInitialized) return "";
        
        std::stringstream dot;
        dot << "digraph MinimizedDFA {" << std::endl;
        dot << "    rankdir=LR;" << std::endl;
        dot << std::endl;
        
        // 找到初始分区
        int initial_partition = -1;
        if (DFAManager::initial_state < DFAMinimizer::state_partition.size()) {
            initial_partition = DFAMinimizer::state_partition[DFAManager::initial_state];
        }
        
        // 确定接受分区
        std::set<int> accept_partitions;
        for (const auto& state : DFAManager::accept_states) {
            if (state < DFAMinimizer::state_partition.size()) {
                accept_partitions.insert(DFAMinimizer::state_partition[state]);
            }
        }
        
        // 先声明起始节点
        dot << "    __start [shape=point];" << std::endl;
        
        // 为每个分区设置正确的形状
        for (int i = 0; i < DFAMinimizer::partitions.size(); i++) {
            if (accept_partitions.find(i) != accept_partitions.end()) {
                dot << "    " << i << " [shape=doublecircle];" << std::endl;
            } else {
                dot << "    " << i << " [shape=circle];" << std::endl;
            }
        }
        
        // 初始状态连接
        if (initial_partition != -1) {
            dot << "    __start -> " << initial_partition << ";" << std::endl;
        }
        dot << std::endl;
        
        // 生成转移边
        for (const auto& [from, symbol, to] : DFAMinimizer::minimized_transitions) {
            dot << "    " << from << " -> " << to;
            dot << " [label=\"" << symbol << "\"];" << std::endl;
        }
        
        dot << "}" << std::endl;
        
        // 调试：保存DOT内容到文件
        std::ofstream debug_file("/tmp/minimized_dfa_debug.dot");
        debug_file << dot.str();
        debug_file.close();
        std::cout << "DEBUG: DOT content saved to /tmp/minimized_dfa_debug.dot" << std::endl;
        
        return dot.str();
    }

    // ...existing code...
}

// 公共API实现
namespace RegexAutomata {
    
    // 自动机构建
    bool buildFromRegex(const std::string& regex) {
        return RegexBuilder::buildAutomataFromRegex(regex);
    }
    
    // 字符串匹配
    bool matchString(const std::string& input) {
        return RegexBuilder::matchString(input);
    }
    
    // 获取自动机描述
    std::string getNFADescription() {
        return RegexBuilder::getNFADescription();
    }
    
    std::string getDFADescription() {
        return RegexBuilder::getDFADescription();
    }
    
    // DOT文件生成（用于可视化）
    std::string generateNFADot() {
        return RegexBuilder::generateNFADotContent();
    }
    
    std::string generateDFADot() {
        return RegexBuilder::generateDFADotContent();
    }
    
    std::string generateMinimizedDFADot() {
        return RegexBuilder::generateMinimizedDFADotContent();
    }
    
    // 调试和信息输出
    void printNFA() {
        NFAManager::print_nfa();
    }
    
    void printDFA() {
        DFAManager::print_dfa();
    }
    
    void printMinimizedDFA() {
        // 实现最小化DFA的打印
        std::cout << "\n=== Minimized DFA ===\n";
        std::cout << "Partitions: " << DFAMinimizer::partitions.size() << "\n";
        for (int i = 0; i < DFAMinimizer::partitions.size(); i++) {
            std::cout << "Partition " << i << ": {";
            bool first = true;
            for (auto state : DFAMinimizer::partitions[i]) {
                if (!first) std::cout << ", ";
                std::cout << state;
                first = false;
            }
            std::cout << "}\n";
        }
        
        std::cout << "Minimized Transitions: ";
        for (const auto& [from, symbol, to] : DFAMinimizer::minimized_transitions) {
            std::cout << "f(" << from << ", " << symbol << ") = " << to << " ";
        }
        std::cout << "\n";
    }
    
    // 自动机状态管理
    void reset() {
        RegexBuilder::resetState();
    }
    
    bool isBuilt() {
        return RegexBuilder::isReady();
    }
}