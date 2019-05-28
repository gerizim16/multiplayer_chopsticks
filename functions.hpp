#pragma once
#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum ClientActions { CLIENT_END,
                     CLIENT_OUTPUT,
                     CLIENT_INPUT };

/* don't include \n in output */
void outputTo(std::ostream *output, std::string line = "") {
    if (output != &std::cout) *output << CLIENT_OUTPUT << std::endl;
    *output << line << std::endl;
}

/* output to all except 3rd param, don't include \n in output */
void outputToAll(std::vector<std::ostream *> outputs, std::string line = "", std::ostream *except = nullptr) {
    for (std::ostream *output : outputs) {
        if (output != except) {
            outputTo(output, line);
        }
    }
}

std::string getlineFrom(std::istream *input, std::ostream *output) {
    std::string result;
    if (input != &std::cin) *output << CLIENT_INPUT << std::endl;
    getline(*input, result);
    return result;
}

bool isValidInt(std::string s) {
    std::string::const_iterator it = s.begin();
    if (*it != '-' && !isdigit(*it)) return false;
    char first_char = *(it++);
    while (it != s.end() && (isdigit(*it))) it++;
    return (first_char == '-' ? s.size() >= 2 : !s.empty()) && it == s.end();
}

bool isValidString(std::string s, int needed) {
    std::stringstream test(s);
    std::string word;

    int count = 0;
    while (test >> word) {
        count++;
    }

    if (count != needed) return false;
    return true;
}

#endif /* FUNCTIONS_HPP */