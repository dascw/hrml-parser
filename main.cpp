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

// @note Set macro to 0 to enable user input
#define TESTING 1


/// @details testing function for parsing input stream / file stream
#if TESTING == 1

    static std::stringstream & getInput() {
        static const std::string cn_TEST_FILE_NAME = "test_4.txt";

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

// Declarations
class Tag; // forward dec required for 'TagPtr' definition
/// @brief TagPtr : smart pointer for tag object storage control
typedef std::unique_ptr<Tag> TagPtr;
///@brief TagLocators : extension of pair for simplistic return
typedef std::pair<std::size_t, std::size_t> TagLocation;


class Tag {
public:
    Tag() {} // default constructor

    /// @brief Tag() : represents an individual tag object
    Tag(const std::string& tag, const std::string& id, const std::string& val) :
        m_tag(tag) {
        m_value[id] = val; // load tree
    }

    /// @brief get() : returns tag string
    /// @return std::string&
    const std::string& get() const { return this->m_tag; }

    /// @brief search() : returns value for given id
    /// @details returns "Not Found!" if id does not exist
    /// @param std::string id
    /// @return std::string value | default string;
    const std::string& search(const std::string& id) {
        assert(id.size() != 0);
        return (m_value.find(id) != m_value.end()) ?
                    m_value[id] : sm_default;
    }

    /// @brief addValues() : set values in hash map
    /// @param std::vector& vals
    void addValues(const std::vector<std::string>& vals) {
        // add all of tag's values
        for (auto idx = vals.begin(); idx != vals.end(); ++idx) {
            // iterate.
            if (*idx == "=")
                this->Tag::setValue(*(idx - 1), *(idx + 1)); // add id, value
        }
    }

    /// @brief getSelf() : returns self ptr for tree traversing
    /// @return Tag* ptr
    Tag * getSelf() {
        return (this);
    }

    /// @brief getSubDiv() : pure virtual
    virtual Tag * getSubDiv(const std::string& tag) const = 0;

    virtual ~Tag () { /*destructor*/ }
    static const std::string sm_default; // allows referenced return from search() method

protected:
    std::string m_tag; // self tag
    std::unordered_map<std::string, std::string> m_value; // allows simplistic look up for any id.

private:
    /// @brief setValue : private method for inserting value into tree
    void setValue(const std::string& id, const std::string& value) {
        assert((id.size() != 0) && (value.size() != 0));
        m_value[id] = value; // allows inserting of values into tree
    }
};

// Init default string
const std::string Tag::sm_default = "Not Found!";

class TagParser {
public:
    /// @brief split() : splits input string on delimiter
    /// @details provides malleable interface for splitting strings on
    /// input delimiter (as well as storing delimiter found).
    /// @param @todo
    std::vector<std::string> split(const std::string& in,
                                    std::size_t& last,
                                    std::pair<std::string, std::string> delim
                                                = std::pair<std::string, std::string>(" ", ">"),
                                    const bool store_delim
                                                = false) const {
        assert((delim.first.size() == 1) && (delim.second.size() == 1));
        assert(in.size() != 0);
        std::vector<std::string> temp; // return object

        for (std::size_t idx = 0; idx < in.size(); ++idx) {

            if ((delim.first[0] == in[idx]) || (delim.second[0] == in[idx])) {
                // space found - construct string, strip equal signs if found.
                std::string temp_str = std::string(in.begin() + last, in.begin() + idx);
                // filter equals sign in value string
                temp_str.erase(std::remove(temp_str.begin(), temp_str.end(), '"'),
                    temp_str.end() );
                temp.push_back(temp_str);

                if (store_delim) // store delimiter if requested
                    temp.push_back(std::string(1, in[idx]));

                last = idx + 1; // update index and step over space

                if (in[idx] == '>') {
                    // exit on div completion
                    break;
                }
            }
        }

        // remove last deliminator (used to force stop the parsing)
        if (store_delim)
            temp.erase(temp.end());

        assert(temp.size() != 0);
        return temp;
    }

protected:
    /// @brief extractDiv() : returns sub string from input string that
    /// contains div body of first tag found in input.
    /// @param std::string : in
    /// @return std::string : out
    std::string extractDiv(const std::string& in) const {
        assert(in.size() != 0);

        const TagLocation tag_loc = findTag(in);

        std::string term = "</" + std::string(in.begin() + (tag_loc.first), in.begin() + tag_loc.second) + ">";
        TagLocation term_loc = TagLocation(in.find(term), 0);
        term_loc.second = term_loc.first + term.size();

        return std::string(in.begin(), in.begin() + term_loc.second);
    }

    explicit TagParser()  {} // default constructor
    virtual ~TagParser()  {} // default destructor

private:
    /// @brief findTag() : returns location of tag open terminator in string
    const TagLocation findTag(const std::string& in) const {
        assert(in.size() != 0);

        TagLocation ret(in.find("<", 0) + 1, 0);  // step over first bracket
        ret.second = ret.first;  // init

        ret.second = in.find_first_of("> ", ret.first); // find tag  terminator

        // protect
        if (ret.second == std::string::npos)
            ret.second = ret.first;

        return ret;
    }
};


class TagDiv final : public Tag, public TagParser {
public:
    TagDiv() {} // default

    /// @brief TagDiv() : constuctor for TagDiv tag tree graph
    TagDiv(const std::string& in) {
        // will be parsed a string which will contain the body of a div
        // the body of the div may contain another TagDiv (i.e. subdiv).

        // split string into substrings through terminator location
        std::vector<std::string> temp;
        std::size_t last = 1;
        temp = this->split(in, last); // sets last = closure of current tag's attributes

        try {

            this->m_tag = temp[0]; // first vector member = tag
            temp.erase(temp.begin());
            this->addValues(temp); // process/update tag values of self
            // if there is still content left in input, we know we have another TagDiv
            // inside this div.
            while (last < in.size()) {
                // Extrapolate div - returns SIZE parsed and result
                std::string div_result =
                        this->extractDiv(std::string(in.begin() + last, in.end() ) );

                if ((div_result.size() > 2) && (div_result[1] != '/')) {
                    // check for empty div and div is not only terminator
                    m_tree.push_back(std::move(std::make_unique<TagDiv>(div_result) ) );
                }

                last += div_result.size(); // move over processed tag.
            }
        } catch(...) {

            // div creation failed (likely from OOR access on temporary vector)
            std::cout << "TagDiv(): creation failed!" <<std::endl;
        }
    }

    /// @brief getSubDiv() : returns pointer to Tag object if
    /// found in self tree
    /// @param tag ID
    /// @return Tag *
    Tag * getSubDiv(const std::string& tag) const {
        Tag * ret_val = nullptr;

        for (auto& a : m_tree) {

            if (a->Tag::get().compare(tag) == 0) {

                ret_val = a->getSelf();
                break;
            }
        }
        return ret_val;
    }

    ~TagDiv() { /*destructor */}
private:
    std::vector<TagPtr> m_tree; // smart pointer tree graph
};

/// @brief Allows simplistic processing of requests
class TagAPIRequest {
public:
    /// @brief TagAPIRequest() : iterator constructor
    /// @details constructs separate string objects
    /// from iterators (all must be of same origin!)
    template<typename T>
    explicit TagAPIRequest(const T& subject,
                            const T& op,
                            const T& val) {
        // construct string based on iterator locations
        this->m_subject  = std::string(subject, op);
        this->m_operator = std::string(op, op + 1); // +1 to step over operator symbol
        this->m_value    = std::string(op + 1, val);
    }

    /// @brief TagAPIRequest() : string constructor
    /// @details allows construction from three indpendent
    /// string ojbects
    explicit TagAPIRequest(const std::string& sub,
                           const std::string& op,
                           const std::string& val) :
        m_subject(sub),
        m_operator(op),
        m_value(val) {
    }

    // Boolean operators
    bool memberOpr()  const { return (m_operator == "."); }
    bool valueOpr()   const { return !memberOpr(); }

    // Get methods
    const std::string& subject()  const { return this->m_subject; }
    const std::string& op()       const { return this->m_operator; }
    const std::string& value()    const { return this->m_value; }

private:
    // members
    std::string m_subject;
    std::string m_operator;
    std::string m_value;
};

class TagAPI {
public:
    TagAPI(std::vector<TagPtr>& head) : m_head(head)  {}

    /// @brief request()<vector> : execute API requests derived from vectored input
    /// @details accepts already parsed and split string
    const std::string request(const std::vector<std::string>& in) const {
        // pre-parsed vector MUST be multiples of 2 + 3 init
        assert((in.size() == 3) || (((in.size() - 3) % 2) == 0));

        std::vector<TagAPIRequest> reqs;
        for (unsigned int idx = 0; idx < in.size() - 1; ) {
            reqs.push_back(TagAPIRequest(in[idx], in[idx + 1], in[idx + 2]));
            idx += 2; // move to next request
        }

        return (this->m_callInit(reqs));
    }

    ~TagAPI() {}
private:
    /// @brief m_callInit() : returns result for given API request queue
    /// @details initial call() method searchs heads (i.e. multiple
    /// graph objects that MAY or may not be interlinked) for the
    /// initial subject. Sets initial base pointer and subsequently
    /// calls recursive method
    /// @return std::string : value
    const std::string m_callInit(std::vector<TagAPIRequest>& queue) const {
        assert(queue.size() != 0);
        Tag* base = nullptr; // set to default

        for (auto& a : m_head) { // for each individual graph node in HRML content
            if (a->Tag::get().compare(queue[0].subject()) == 0) {
                base = a.get(); // update
                break;
            }
        }

        return (this->m_callRecursive(base, queue));
    }

    /// @brief m_callRecursive() : member method for recursive API iteration
    /// @details deletes queue object, base pointer is not modified
    /// @return std::string : value
    const std::string m_callRecursive(Tag* base, std::vector<TagAPIRequest>& queue) const {

        if (queue.empty() == false) {
            auto temp = *(queue.begin()); // temp storage for processing
            queue.erase(queue.begin()); // pop

            if ((base != nullptr) && (base->Tag::get().compare(temp.subject()) == 0)) {
                return (temp.memberOpr() ? this->m_callRecursive(base->getSubDiv(temp.value()), queue) :
                                          base->Tag::search(temp.value()));
            }
        }

        return Tag::sm_default;
    }

private:
    const std::vector<TagPtr>& m_head;  // reference set in derived class on instantiation
                                        // @note not modified through API
};

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
    std::string full = std::accumulate(lines.begin(), lines.end(), std::string(""));

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
