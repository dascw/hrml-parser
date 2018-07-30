#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <streambuf>
#include <fstream>
#include <sstream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <assert.h>

#include "hrml_parse.h"

// @note Set macro to 0 to enable user input (hackerrank parse).
#define TESTING 1

/// @details testing function for parsing input stream / file stream
#if TESTING == 1

    static std::stringstream & getInput() {
        static const std::string cn_TEST_FILE_NAME = "test/test_4.txt";

        static bool init = false;
        static std::stringstream buf;
        if (init == false) {
            std::ifstream file(cn_TEST_FILE_NAME);
            if (file) { // opened
                buf << file.rdbuf();
                file.close();
            } else {
                throw "File read failed!"; // except on char* throw
            }
        }

        return buf;
    }
#else
    stataic std::istream& getInput() {
        return std::cin;
    }

#endif // TESTTING

int main() {
    // required for vector size generation
    int line_num;
    int req_num;

    // Retrieve line numbers
    getInput() >> line_num >> req_num;
    // Build vectors for lines/API requests
    std::vector<std::string> lines(line_num);
    std::vector<std::string> request(req_num);

    /// @brief lambda for parsing user/file input
    auto retrieve = [&](std::vector<std::string>& store)->void {
        for (std::size_t idx = 0; idx < store.size(); ) {
            std::getline(getInput(), store[idx]);
            // When parsing test data, line terminator can spuriously push vector
            // along : check contents is not empty blocks this.
            if (store[idx].compare("") != 0)
                idx++;
        }
    };

    // Retrieve contents from user-input/file
    retrieve(lines);
    retrieve(request);

    // build full string for processing (whole stream required to generate tree recurisvely)
    std::string full = std::accumulate(lines.begin(), 
                                        lines.end(), 
                                        std::string(""));

    do {
        // parse constructed content
        HRMLParser::action().HRMLParser::init(full);

        // Process all requests to div content.
        for (auto& a : request) {
            // Print result of API request - "Not Found!" returned if invalid request
            std::size_t temp_size = 0; // required due to modified split function
            auto req_split = HRMLParser::action().TagParser::split(a + ".", temp_size, std::pair<std::string, std::string>(".", "~"), true);
            std::cout << HRMLParser::action().TagAPI::request(req_split) << std::endl;
        }
    } while (0); // scope reduction

    return 0;
}

/************************************end of file************************************/
