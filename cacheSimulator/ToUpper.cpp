// main.cpp
#include <iostream>
#include <string>
#include <algorithm>

int main() {
    std::string input;
    std::getline(std::cin, input);
    // std::transform(input.begin(), input.end(),input.begin(), ::toupper); 
    std::cout << input; 
    return 0;
}
