/**

file

author Mattia Basaglia

section License

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

#ifndef RE_PARSER_HPP
#define RE_PARSER_HPP

#include <istream>
#include <string>

#include "re_nfa.hpp"

namespace regex {

namespace ast { class Node; }

class Parser {
public:
    virtual ~Parser() {}

    /**
     * \brief Compiles the input stream
     * \return The corresponding automaton
     */
    nfa::NFA compile(std::istream& input);

    /**
     * \brief Compiles the input string
     * \return The corresponding automaton
     */
    nfa::NFA compile(const std::string& input);

    /**
     * \brief Whether the parser has encountered an error
     */
    virtual bool error() const = 0;

protected:
    /**
     * \brief Parses the input stream
     * \return A dynamically-allocated AST tree
     */
    virtual ast::Node* parse(std::istream& input) = 0;
};

class SimpleParser : public Parser {
public:
    bool error() const override;

protected:
    ast::Node* parse(std::istream& input) override;

private:
    ast::Node* parse_choice(std::istream& input) const;
    ast::Node* parse_concat(std::istream& input) const;
    /**
     * \pre  input.peek() is ( or a character indicating some kind of character match
     * \post input.peek() is the character follwing the primary expression
     */
    ast::Node* parse_primary(std::istream& input) const;
    /**
     * \pre  input.peek() is the character following the [
     * \post input.peek() is the character following the ]
     */
    ast::Node* parse_bracket(std::istream& input) const;
    /**
     * \pre  input.peek() is the quantifier character
     * \post input.peek() is the character following the quantifier
     * \note returns \c child if there is no quantifier
     */
    ast::Node* parse_quantifier(std::istream& input, ast::Node* child) const;

    bool is_primary(std::char_traits<char>::int_type c) const;
};

} // namespace regex

#endif // RE_PARSER_HPP
