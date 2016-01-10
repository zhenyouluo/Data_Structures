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

#ifndef RE_AST_HPP
#define RE_AST_HPP

#include "re_nfa.hpp"

namespace regex {

/**
 * \brief Intermediate representation of RegEx parsing
 */
namespace ast {

/**
 * \brief Abstract Node class
 * \note Deleting copy, add clone if needed
 */
class Node {
public:
    Node(){}
    virtual ~Node(){}
    /**
     * \brief Build an automaton. It must always contain an input and output node.
     */
    virtual nfa::NFA build() const = 0;

private:
    Node (const Node&) = delete;
    Node& operator= (const Node&) = delete;
};

/**
 * \brief Node with a single child (abstract). Takes ownersip
 */
class UnaryNode : public Node {
public:

    explicit UnaryNode (Node* child)
        : child(child) {}

    ~UnaryNode() {
        delete child;
    }

protected:
    Node* child;
};

/**
 * \brief Node with two children (abstract). Takes ownersip
 */
class BinaryNode : public Node {
public:

    explicit BinaryNode (Node* left, Node* right)
        : left(left), right(right) {}

    ~BinaryNode() {
        delete left;
        delete right;
    }

protected:
    Node* left;
    Node* right;
};

/**
 * \brief /a|b/
 */
class Choice : public BinaryNode {
public:
    using BinaryNode::BinaryNode;
    nfa::NFA build() const override;
};

/**
 * \brief /ab/
 */
class Concat : public BinaryNode {
public:
    using BinaryNode::BinaryNode;
    nfa::NFA build() const override;
};

/**
 * \brief /a* /
 */
class KleeneStar : public UnaryNode {
public:
    using UnaryNode::UnaryNode;
    nfa::NFA build() const override;
};

/**
 * \brief /a+/
 */
class KleenePlus : public UnaryNode {
public:
    using UnaryNode::UnaryNode;
    nfa::NFA build() const override;
};

/**
 * \brief /a?/
 */
class Optional : public UnaryNode {
public:
    using UnaryNode::UnaryNode;
    nfa::NFA build() const override;
};

/**
 * \brief /(a)/ Needed if maybe I want to capture them.
 */
class Subexpression : public UnaryNode {
public:
    using UnaryNode::UnaryNode;
    nfa::NFA build() const override;
};

/**
 * \brief Leaf, implements a simple transition
 */
class Leaf : public Node {
public:
    Leaf(const nfa::Transition::Functor& condition);
    nfa::NFA build() const override;
private:
    nfa::Transition::Functor condition;
};

/**
 * \brief Single character transition
 */
class SingleCharacter : public Leaf {
public:
    SingleCharacter(char c);
};


} } // namespace regex::ast

#endif // RE_AST_HPP
