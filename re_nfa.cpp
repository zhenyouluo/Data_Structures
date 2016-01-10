/**

\file

\author Mattia Basaglia

\section License

Copyright (C) 2015-2016  Mattia Basaglia

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

#include "re_nfa.hpp"

using namespace regex::nfa;

NFA::NFA() {
    set_input(new Node);
    set_output(new Node);
}

NFA::NFA(NFA &&other)
        : input_(other.input_), output_(other.output_) {
    acquire_nodes(other);
}

NFA &NFA::operator=(NFA &&other) {
    clear();

    input_ = other.input_;
    output_ = other.output_;

    acquire_nodes(other);

    return *this;
}

NFA NFA::duplicate() const {
    std::unordered_map<Node*, Node*> translation;

    NFA result;

    for (Node* node : nodes_) {
        Node* copied_node = node->clone();
        translation[node] = copied_node;
        result.insert_node(copied_node);
    }

    for (Node* node : result.nodes_) {
        node->translate(translation);
    }

    return std::move(result);
}

NFA::~NFA() {
    clear();
}

void NFA::insert_node(Node *node) {
    if (node->graph_ != this) {
        if (node->graph_)
            node->graph_->remove_node(node);
        node->graph_ = this;
        nodes_.insert(node);
    }
}

void NFA::remove_node(Node *node) {
    if (node->graph_ == this) {
        node->graph_ = nullptr;
        if (node == input_)
            input_ = nullptr;
        if (node == output_)
            output_ = nullptr;
        nodes_.erase(node);
    }
}

void NFA::clear() {
    input_ = output_ = nullptr;
    for (auto node : nodes_) {
        node->graph_ = nullptr;
        delete node;
    }
    nodes_.clear();
}

NodeList NFA::nodes() const {
    return nodes_;
}

void NFA::make_deterministic() {
    // TODO
}

bool NFA::merge(NFA &other) {
    if (&other == this || !output_ || !other.input_ || !other.output_)
        return false;

    output_->merge(*other.input_);
    delete other.input_;
    set_output(other.output_);
    acquire_nodes(other);

    return true;
}

void NFA::acquire_nodes(NFA &other)
{
    other.input_ = other.output_ = nullptr;
    for (auto node : other.nodes_) {
        node->graph_ = this;
    }
    nodes_.insert(other.nodes_.begin(),other.nodes_.end());
    other.nodes_.clear();
}

Node *NFA::input() const {
    return input_;
}

void NFA::set_input(Node *node) {
    insert_node(node);
    input_ = node;
}

Node *NFA::output() const {
    return output_;
}

void NFA::set_output(Node *node) {
    insert_node(node);
    output_ = node;
}


Node::Node(NFA *graph) : graph_(graph) {}

Node::~Node() {
    if (graph_)
        graph_->remove_node(this);
}

Node *Node::clone() const {
    return new Node(*this);
}

void Node::translate(const std::unordered_map<Node *, Node *> &map) {
    NodeList old_empty_transitions = empty_transitions_;
    empty_transitions_.clear();
    for (Node* node : old_empty_transitions) {
        auto it = map.find(node);
        if (it != map.end())
            empty_transitions_.insert(it->second);
    }

    auto old_transitions = transitions_;
    transitions_.clear();
    for (const Transition& transition: old_transitions) {
        auto it = map.find(transition.target);
        if (it != map.end())
            transitions_.push_back({it->second,transition.condition});
    }
}

void Node::add_transition(Node *target) {
    if (target)
        empty_transitions_.insert(target);
}

void Node::add_transition(Node *target, const Transition::Functor &condition) {
    add_transition({target,condition});
}

void Node::add_transition(const Transition &transition) {
    if (transition.condition && transition.target)
        transitions_.push_back(transition);
}

NodeList Node::empty_transitions() const {
    return empty_transitions_;
}

NodeList Node::next_nodes(char c) const {
    NodeList nodes;
    for (const auto& transition : transitions_)
        if (transition.condition(c))
            nodes.insert(transition.target);
    return nodes;
}

NFA *Node::graph() const {
    return graph_;
}

void Node::merge(const Node &other) {
    transitions_.insert(transitions_.end(),other.transitions_.begin(),other.transitions_.end());
    empty_transitions_.insert(other.empty_transitions_.begin(),other.empty_transitions_.end());
}

NfaRunner::NfaRunner(const NFA& nfa) : nfa_(nfa) {
    set_state({nfa.input()});
}

const NodeList& NfaRunner::state() const {
    return state_;
}

void NfaRunner::set_state(const NodeList& state) {
    state_ = expand_empty(state);
}

bool NfaRunner::acceptable() const {
    return nfa_.output() && state_.count(nfa_.output());
}

void NfaRunner::step(char c) {
    state_ = expand(state_,c);
}

NodeList NfaRunner::expand_empty(NodeList source) {
    NodeList output;
    while (!source.empty()) {
        Node* current_node = *source.begin();
        source.erase(current_node);
        if (!current_node)
            continue;
        output.insert(current_node);
        for (Node* adjacent : current_node->empty_transitions()) {
            if (!source.count(adjacent) && !output.count(adjacent))
                source.insert(adjacent);
        }
    }
    return output;
}

NodeList NfaRunner::expand(NodeList source, char c) {
    NodeList output;
    for (Node* source_node : source)
        for (Node* output_node : source_node->next_nodes(c))
            if (!output.count(output_node))
                output.insert(output_node);
    return expand_empty(output);
}
