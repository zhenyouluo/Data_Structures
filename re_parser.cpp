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

#include "re_parser.hpp"

#include <sstream>

#include "re_ast.hpp"

using namespace regex;

nfa::NFA regex::Parser::compile(std::istream &input) {
    ast::Node* parsed = parse(input);
    if (!parsed)
        return nfa::NFA();
    nfa::NFA nfa = std::move(parsed->build());
    delete parsed;
    return std::move(nfa);
}

nfa::NFA Parser::compile(const std::string &input) {
    std::istringstream ss(input);
    return std::move(compile(ss));
}

bool SimpleParser::error() const {
    return false;
}

ast::Node *SimpleParser::parse(std::istream &input) {
    if (!input)
        return nullptr;
    return parse_choice(input);
}

ast::Node *SimpleParser::parse_choice(std::istream &input) const {
    ast::Node* node = parse_concat(input);
    while (input.peek() == '|') {
        input.get();
        if (input.peek() == std::char_traits<char>::eof())
            break;
        node = new ast::Choice(node, parse_concat(input));
    }
    return node;
}

ast::Node *SimpleParser::parse_concat(std::istream &input) const {
    ast::Node* node = parse_primary(input);
    while (is_primary(input.peek())) {
        node = new ast::Concat(node,parse_primary(input));
    }
    return node;
}

ast::Node *SimpleParser::parse_primary(std::istream &input) const {
    std::char_traits<char>::int_type c = input.get();
    ast::Node *node = nullptr;
    if (c == '\\' && input.peek() != std::char_traits<char>::eof()) {
        node = new ast::SingleCharacter(input.get());
    } else if (c == '.') {
        node = new ast::Leaf([](char){return true;});
    } else if (c == '[') {
        node = parse_bracket(input);
    } else if (c == '(') {
        bool capture = true;
        if (input.peek() == '?') {
            input.get();
            if (input.peek() == ':') {
                input.get();
                capture = false;
            }
        }
        node = parse_choice(input);
        if (capture)
            node = new ast::Subexpression(node);
        input.get(); // skip )
    } else {
        node = new ast::SingleCharacter(c);
    }

    return parse_quantifier(input, node);
}

ast::Node *SimpleParser::parse_bracket(std::istream &input) const {
    std::char_traits<char>::int_type c = input.get();
    bool negate = false;
    std::string characters;
    if ( c == '^' ) {
        negate = true;
        c = input.get();
    } else if ( c == '-' ) {
        characters += c;
        c = input.get();
    }

    while ( c != std::char_traits<char>::eof() && c != ']' ) {
        // TODO: [:alpha:]
        if (c == '\\' && input.peek() != std::char_traits<char>::eof()) {
            characters += input.get();
        } else if (input.peek() == '-') {
            input.get();
            char d = input.get();
            for (; c <= d; c++)
                characters += c;
        } else {
            characters += c;
        }
        c = input.get();
    }
    if ( negate )
        return new ast::Leaf([characters](char c) { return characters.find(c) == std::string::npos; });
    return new ast::Leaf([characters](char c) { return characters.find(c) == std::string::npos; });
}

ast::Node *SimpleParser::parse_quantifier(std::istream &input, ast::Node *child) const {
    std::char_traits<char>::int_type c = input.get();
    if (c == '?')
        return new ast::Optional(child);
    else if (c == '*')
        return new ast::KleeneStar(child);
    else if (c == '+')
        return new ast::KleenePlus(child);
    // TODO: a{3,5}
    if (c != std::char_traits<char>::eof())
        input.unget();
    return child;
}

bool SimpleParser::is_primary(std::char_traits<char>::int_type c) const {
    return c != ')' && c != '|' && c != std::char_traits<char>::eof();
}
