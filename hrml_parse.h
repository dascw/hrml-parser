/**
 * @brief hyper-redundant-markup-language parser. 
 * @details HRML is roughly equivalent to HTML, however it's redundant, and only
 * beneficial for this excercise. HRML has a unique query method for getting values
 * from built tree. 
 * 
 * @file hrml_parse.h
 * @author SCW
 */
#ifndef HRML_PARSE_H
#define HRML_PARSE_H

#include <string>
#include <stdint.h>

#include "tag_api.h"

/// @brief HRMLParser : front end for parsing / accessing HRML content
/// @details inforced singleton object
class HRMLParser final : public TagParser, public TagAPI::Interface {
public:
    /// @brief action() : access method for acting on singleton object
    static HRMLParser * GetInstance() {
        static HRMLParser * m_instance = nullptr;
        
        if (m_instance == nullptr) 
            m_instance = new HRMLParser();
            
        return m_instance;
    }

    /// @brief init() : method to initialise parser
    /// @param std::string& : concatenation of user input
    void Init(const std::string& full) {
        std::string sub = full;
        do {
            // Create div up to end of div object (e.g. <tag>...</tag>).
            std::string div = TagParser::ExtractDiv(sub);

            // Parse the current div object.
            m_head.push_back(std::make_unique<TagDiv>(div));

            // resize sub string to remove all processed contents.
            sub.erase(sub.begin(), sub.begin() + div.size());

            m_div_end += div.size(); // update locator to track end of input

        } while (m_div_end < full.size()); // Check if the entire input has been parsed, if not, div object still likely remains.
    }

    HRMLParser(HRMLParser const&)       = delete;
    void operator=(HRMLParser const&)   = delete;
private:
    /// @brief constructor() : forced private to prevent multiple instances (singleton)
    HRMLParser()
        : TagAPI::Interface(m_head), m_div_end(0) { }

    ~HRMLParser() {}

    std::vector<TagPtr> m_head; // vector of smart pointers to div objects
    std::size_t m_div_end;      // end of current div
};

#endif 
    // HRML_PARSE_H
/******************************************end of file******************************************/
