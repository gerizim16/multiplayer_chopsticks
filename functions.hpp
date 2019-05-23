#pragma once
#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

const int CLIENT_END = -1;
const int CLIENT_OUTPUT = 0;
const int CLIENT_INPUT = 1;

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

bool is_valid_int(std::string s) {
    std::string::const_iterator it = s.begin();
    if (*it != '-' && !isdigit(*it)) return false;
    while (it != s.end() && (isdigit(*it))) it++;
    return !s.empty() && it == s.end();
}

bool is_valid_string(std::string s, int needed){
    std::stringstream test(s);
    std::string word;

    int count = 0;
    while (test >> word){
        count++;
    }

    if (count != needed) return false;
    return true;
}

#endif /* FUNCTIONS_HPP */