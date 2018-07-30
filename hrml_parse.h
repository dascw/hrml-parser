// hrml_parser.hrml_parser
#ifndef HRML_PARSE_H
#define HRML_PARSE_H

#include <string>
#include <stdint.h>

#include "tag_api.h"

/// @brief HRMLParser : front end for parsing / accessing HRML content
/// @details inforced singleton object
class HRMLParser final : public TagParser, public TagAPI {
public:
    /// @brief action() : access method for acting on singleton object
    static HRMLParser& action() {
        static HRMLParser s_instance; // singleton instance of HRMLParser

        return s_instance;
    }

    /// @brief init() : method to initialise parser
    /// @param std::string& : concatenation of user input
    void init(const std::string& full) {
        std::string sub = full; // copy string for sub processing
        do {
            // Create div up to end of div object (e.g. <tag>...</tag>).
            std::string div = TagParser::extractDiv(sub);

            // Parse the current div object.
            m_head.push_back(std::make_unique<TagDiv>(div));

            // resize sub string to remove all processed contents.
            sub.erase(sub.begin(), sub.begin() + div.size());

            m_div_end += div.size(); // update locator to track end of input

        } while (m_div_end < full.size()); // Check if the entire input has been parsed,
                                           // if not, div object still likely remains.
    }

    // force remove constructors (c++11 required)
    HRMLParser(HRMLParser const&)       = delete;
    void operator=(HRMLParser const&)   = delete;
private:
    /// @brief constructor() : forced private to prevent multiple instances (singleton)
    HRMLParser()
        : TagAPI(m_head), m_div_end(0) { }

    ~HRMLParser() {}

    std::vector<TagPtr> m_head; // vector of smart pointers to div objects
    std::size_t m_div_end;      // end of current div
};

#endif 
    // HRML_PARSE_H
    /******************************************end of file******************************************/