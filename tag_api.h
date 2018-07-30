#ifndef TAG_API_H
#define TAG_API_H
// tag_api.h

#include <string>
#include <vector>

#include "tag.h"

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

#endif 
    // TAG_API_H

/************************************end of file************************************/