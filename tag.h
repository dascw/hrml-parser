// tag.h
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <assert.h>

// Aliases
///@brief TagLocators : extension of pair for simplistic return
using TagLocation = std::pair<std::size_t, std::size_t>;
class Tag;
/// @brief TagPtr : smart pointer for tag object storage control
using TagPtr = std::unique_ptr<Tag>;

class Tag {
public:
    Tag() = default;

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
                    m_value[id] : defaultString();
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

    static const std::string& defaultString() {
        static const std::string default_string("Not Found!");
        return default_string;
    }

    virtual ~Tag () = default;
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

    explicit TagParser() = default;
    virtual ~TagParser() = default;
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
    TagDiv() = default;

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

    ~TagDiv() = default;
private:
    std::vector<TagPtr> m_tree; // smart pointer tree graph
};

/**************************************end of file**************************************/
