#include "converter.h"
#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "1. SHP to Lanelet2" << std::endl;
    std::cout << "2. Lanelet2 to SHP" << std::endl;
    std::cout << "Please select (1 or 2): ";
}

int main() {
    Converter converter;
    int choice;
    std::string inputFile, outputFile;

    printUsage();
    std::cin >> choice;

    if (choice == 1) {
        std::cout << "Enter input SHP file path: ";
        std::cin >> inputFile;
        std::cout << "Enter output Lanelet2 file path (*.osm): ";
        std::cin >> outputFile;

        if (converter.shpToLanelet2(inputFile, outputFile)) {
            std::cout << "Successfully converted SHP to Lanelet2!" << std::endl;
        }
        else {
            std::cout << "Conversion failed!" << std::endl;
        }
    }
    else if (choice == 2) {
        std::cout << "Enter input Lanelet2 file path (*.osm): ";
        std::cin >> inputFile;
        std::cout << "Enter output SHP file path: ";
        std::cin >> outputFile;

        if (converter.lanelet2ToShp(inputFile, outputFile)) {
            std::cout << "Successfully converted Lanelet2 to SHP!" << std::endl;
        }
        else {
            std::cout << "Conversion failed!" << std::endl;
        }
    }
    else {
        std::cout << "Invalid choice!" << std::endl;
        return 1;
    }

    return 0;
}