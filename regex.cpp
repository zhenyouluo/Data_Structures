/**
\file

\author Mattia Basaglia

\section License

Copyright (C) 2014  Mattia Basaglia

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "regex.hpp"
#include "re_nfa.hpp"

using namespace regex;

RegEx::RegEx(const std::string &expression) {
    set_expression(expression);
}

bool RegEx::full_match(const std::string &string) {
    if (!compiled_)
        compile();
    // TODO run the NFA and check if at the end we reached the NFA output and the end of the string
    return false;
}

void RegEx::set_expression(const std::string &expression) {
    expression_ = expression;
    compiled_.reset();
}

std::string RegEx::expression() const {
    return expression_;
}

nfa::NFA* RegEx::compiled() const {
    nfa::NFA* automaton = new nfa::NFA;
    // TODO Parser which creates an AST and move build to automaton
    return automaton;
}

void RegEx::compile() {
    compiled_.reset(compiled());
}
