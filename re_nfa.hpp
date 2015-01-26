/**

\file

\author Mattia Basaglia

\section License

Copyright (C) 2015  Mattia Basaglia

This is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef RE_NFA_HPP
#define RE_NFA_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace regex {
namespace nfa {

class Node;
class Transition;

typedef std::unordered_set<Node*> NodeList;

/**
 * \brief Undeterministic automaton, used to execute regular expressions
 */
class NFA {
public:
    /**
     * @brief Creates a NFA with an input and output node but no transitions
     */
    NFA();
    NFA(NFA&& other);
    NFA(const NFA&) = delete;
    NFA& operator=(NFA&& other);
    NFA& operator=(const NFA&) = delete;

    /**
     * @brief Explicit copy
     */
    NFA duplicate() const;

    /**
      * Deletes all nodes
      */
    virtual ~NFA();

    /**
     * \brief Inserts a node to the graph (takes ownership)
     * \param node
     */
    void insert_node(Node* node);

    /**
     * \brief If the node is contained in this graph, removes it without deleting it
     * \param node
     */
    void remove_node(Node* node);

    /**
     * @brief Remove and delete all nodes (including input and output)
     */
    void clear();

    /**
     * \brief Returns the list of nodes
     */
    NodeList nodes() const;

    /**
     * \brief Turn the automaton into an equivalent deterministic automaton
     */
    void make_deterministic();

    /**
     * @brief Merges another graph to the current one, emptying it in the process
     * @return Whether the merge was successful
     */
    bool merge(NFA& other);

    /**
     * @brief Moves all the nodes from other to this
     */
    void acquire_nodes(NFA& other);

    /**
     * @brief Input node
     */
    Node* input() const;

    /**
     * @brief Set input node
     */
    void set_input(Node* node);

    /**
     * @brief Output node
     */
    Node* output() const;

    /**
     * @brief Set output node
     */
    void set_output(Node* node);

private:
    NodeList nodes_;
    Node* input_ = nullptr;
    Node* output_ = nullptr;
};


/**
 * @brief Transition between nodes, takes a character and returns if it matches
 */
class Transition {
public:
    typedef std::function<bool(char)> Functor;

    Transition(Node* target, const Functor& condition)
        : target(target), condition(condition) {}

    Node*   target;
    Functor condition;
};

/**
 * @brief Automaton node
 */
class Node {
public:

    explicit Node(NFA* graph = nullptr);
    virtual ~Node();

    /**
     * @brief Polymorphic copy
     * @note It will keep intact references to other nodes
     */
    virtual Node* clone() const;

    /**
     * @brief Translates references to other nodes
     */
    void translate(const std::unordered_map<Node*,Node*>& map);

    /**
     * @brief Adds an empty transition
     */
    void add_transition(Node* target);

    /**
     * @brief Adds a transition
     */
    void add_transition(Node* target, const Transition::Functor& condition);

    /**
     * @brief Adds a transition
     */
    void add_transition(const Transition& transition);

    /**
     * @brief Returns nodes immediately reachable from this node
     */
    NodeList empty_transitions() const;

    /**
     * @brief Returns the nodes directly accessible trhough the given character.
     * (It doesn't expand empty transitions)
     */
    NodeList next_nodes(char c) const;

    /**
     * @brief Returns the containing graph
     */
    NFA* graph() const;

    /**
     * @brief Merge this node with another
     */
    void merge(const Node& other);

private:
    std::vector<Transition> transitions_;
    NodeList                empty_transitions_;
    NFA*                    graph_ = nullptr;

    friend class NFA;
};

} } // namespace regex::dfa
#endif // RE_DFA_HPP
