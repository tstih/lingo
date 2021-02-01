#include <algorithm>

#include "rule.hpp"

namespace lingo {

    rule rule::operator| (const rule& l) { 
        return chain<or_node>(l);
    }

    rule rule::operator+ (const rule& l) { 
        return chain<next_node>(l);
    }

    rule rule::operator! () { 
        rule pnt;
        pnt.node_=std::make_shared<not_node>();
        pnt.node_->children_.push_back(node_);
        return pnt;
    }

    template <class T>
    rule rule::chain(const rule &l) {
        rule pnt; // Create parent rule.
        pnt.node_=std::make_shared<T>();

        // Is it an or operator? If yes, just add to enable chaining.
        if(std::dynamic_pointer_cast<T>(node_)!=nullptr) 
            copy(
                node_->children_.begin(), 
                node_->children_.end(), 
                back_inserter(pnt.node_->children_));
        else 
            pnt.node_->children_.push_back(node_);

        pnt.node_->children_.push_back(l.node_);

        return pnt;
    }
}