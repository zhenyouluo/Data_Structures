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

#include "re_ast.hpp"

using namespace regex::nfa;
using namespace regex::ast;

NFA Choice::build() const {
    NFA graph;

    if (left) {
        NFA left_graph = std::move(left->build());
        graph.input()->add_transition(left_graph.input());
        left_graph.output()->add_transition(graph.output());
        graph.acquire_nodes(left_graph);
    }

    if (right) {
        NFA right_graph = std::move(right->build());
        graph.input()->add_transition(right_graph.input());
        right_graph.output()->add_transition(graph.output());
        graph.acquire_nodes(right_graph);
    }

    return std::move(graph);
}


NFA Concat::build() const {
    if (left && right) {
        NFA left_graph = std::move(left->build());
        NFA right_graph = std::move(right->build());
        left_graph.merge(right_graph);
        return std::move(left_graph);
    }
    return NFA();
}


NFA KleeneStar::build() const {
    if (!child)
        return NFA();

    NFA result;

    NFA child_graph = std::move(child->build());

    result.input()->add_transition(result.output());
    result.input()->add_transition(child_graph.input());
    child_graph.output()->add_transition(child_graph.input());
    child_graph.output()->add_transition(result.output());
    result.acquire_nodes(child_graph);

    return std::move(result);
}


NFA KleenePlus::build() const {
    if (!child)
        return NFA();

    NFA result;

    NFA child_graph = std::move(child->build());

    result.input()->add_transition(child_graph.input());
    child_graph.output()->add_transition(child_graph.input());
    child_graph.output()->add_transition(result.output());
    result.acquire_nodes(child_graph);

    return std::move(result);
}


NFA Optional::build() const {
    if (!child)
        return NFA();

    NFA child_graph = std::move(child->build());
    child_graph.input()->add_transition(child_graph.output());
    return std::move(child_graph);
}


NFA Subexpression::build() const {
    if (!child)
        return NFA();
    // NOTE it doesn't actually do anything
    return std::move(child->build());
}


Leaf::Leaf(const Transition::Functor &condition)
    : condition(condition) {}

NFA Leaf::build() const {
    NFA graph;
    graph.input()->add_transition(graph.output(),condition);
    return std::move(graph);
}


SingleCharacter::SingleCharacter(char c)
    : Leaf([c](char d){return c == d;}) {}
