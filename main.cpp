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

/// @brief class for parsing input stream / file stream
/// @details singleton class.
class InputParse {
public:
    static InputParse * GetInstance() {
        if (m_instance == nullptr)
            m_instance = new InputParse();
        return m_instance;
    }
#if TESTING == 1
    static std::stringstream& GetInput() {
        return m_buf;
    }
#else
    stataic std::istream& GetInput() {
        return std::cin;
    }
#endif // TESTING
private:
    InputParse()  { 
    #if TESTING == 1
        static const std::string cn_TEST_FILE_NAME("test/test_4.txt");
        std::ifstream file(cn_TEST_FILE_NAME);
        if (file) {
            m_buf << file.rdbuf();
            file.close();
        } else {
            throw "file read failed!"; // exception
        }
    #endif // TESTING
    }

    ~InputParse() = default;

    static InputParse * m_instance;
    static std::stringstream m_buf;
};
InputParse * InputParse::m_instance = nullptr;
std::stringstream InputParse::m_buf;



int main() {
    // required for vector size generation
    int line_num;
    int req_num;

    InputParse::GetInstance()->GetInput() >> line_num >> req_num;

    // Build vectors for lines/API requests
    std::vector<std::string> lines(line_num);
    std::vector<std::string> request(req_num);

    /// @brief lambda for parsing user/file input
    auto retrieve = [&](std::vector<std::string>& store)->void {
        for (std::size_t idx = 0; idx < store.size(); ) {
            std::getline(InputParse::GetInstance()->GetInput(), store[idx]);
            // When parsing test data, line terminator can spuriously push vector
            // along; checking contents is not empty blocks this.
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
