#ifndef _RULE_HPP
#define _RULE_HPP

#include <memory>

#include "node.hpp"

namespace lingo {

    class rule {
    public:
        rule(char from, char to) {node_ = std::make_shared<literal>(from,to);}
        rule(char c) {node_ = std::make_shared<literal>(c);}
        rule(std::initializer_list<char> l) { 
            node_=std::make_shared<or_node>();
            for (char c : l) 
                node_->children_.push_back(std::make_shared<literal>(c)); 
        }
        rule(std::string s) {
            node_=std::make_shared<next_node>();
            for (char c : s)
                node_->children_.push_back(std::make_shared<literal>(c));
        }

        virtual ~rule()=default;

        rule operator| (const rule& l);
        rule operator+ (const rule& l);
        rule operator! ();

        void print() { node_->print(); }

    protected:
        template <class T> rule chain(const rule &l);

    private:
        rule() {} // Empty rule ctor.
        std::shared_ptr<node> node_;


    };
}

#endif // _RULE_HPP