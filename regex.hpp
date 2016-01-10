/**
\file

\author Mattia Basaglia

\section License

Copyright (C) 2014-2016  Mattia Basaglia

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
#ifndef REGEX_HPP
#define REGEX_HPP

#include <memory>
#include <string>

/**
 * \brief Namespace for stuff regarding regular expressions
 */
namespace regex {

namespace nfa { class NFA; }
class Parser;

class RegEx {
public:
    explicit RegEx(const std::string& expression, std::shared_ptr<Parser> parser = nullptr);

    /**
     * \brief Checks if the entire string matches the regular expression
     */
    bool full_match(const std::string &string);

    void set_expression(const std::string& expression);
    std::string expression() const;

protected:
    /**
     * \brief Compiles the regular expression
     * \return A pointer with the compiled expression
     */
    nfa::NFA *compiled() const;
    /**
     * \brief Compiles the regular expression and stores the result
     */
    void compile();

private:
    std::string expression_;
    std::shared_ptr<nfa::NFA> compiled_;
    std::shared_ptr<Parser> parser_;
};

} // namespace regex

#endif // REGEX_HPP
