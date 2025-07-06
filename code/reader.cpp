#include "header.h"
#include <sstream>
#include <fstream>
#include <string>

bool IntReader(std::ifstream &input, int &num) {
    if (input.eof()) {
        return false;
    }
    std::string s;
    std::getline(input, s);
    bool tag = true;
    for (auto c : s) {
        if (c >= '0' && c <= '9') {
            tag = false;
            break;
        }
    }
    if (tag) {
        return false;
    }
    std::stringstream ss(s);
    ss >> num;
    return true;
}