#ifndef _LINGO_HPP
#define _LINGO_HPP

#include <vector>
#include <memory>
#include <iostream>

namespace lingo {


    // ----- nodes ------------------------------------------------------------
    class node {
    public:
        virtual void print(int indent=0)=0;
    };

    class unary_node : public node { // Just 1 child.
    public:
        virtual void print(int indent)=0;
    protected:
        friend class rule;
        std::shared_ptr<node> children_;
    };

    class multary_node : public node { // N children.
    public:
        virtual void print(int indent)=0;
    protected:
        friend class rule;
        std::vector<std::shared_ptr<node>> children_;
    };

    class literal : public node { // Special node, without children.
    public:
        literal(char c) : from_(c), to_(c) {}
        literal(char from, char to) : from_(from), to_(to) {}
        void print(int indent) override { 
            std::cout << std::string(indent, ' ') << "literal (" << from_ << "," << to_ << ")" <<  std::endl; }
    private:
        char from_;
        char to_;
    };

    class or_node : public multary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "or" << std::endl; for(auto n:children_) n->print(indent + 1); }
    };

    class next_node : public multary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "next" << std::endl; for(auto n:children_) n->print(indent + 1); }
    };

    class not_node : public unary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "not" << std::endl; for(auto n:children_) n->print(indent + 1); }    
    };

    class repeat_node : public unary_node {
    public:
        repeat_node(int min=1, int max=0) : min_(min), max_(max) {}
        void print(int indent) override { std::cout << std::string(indent, ' ') << "repeat (" << min_ << "," << max_ << ")" << std::endl; for(auto n:children_) n->print(indent + 1); } 
    private:
        int min_;
        int max_;
    };

    class optional_node : public unary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "optional" << std::endl; for(auto n:children_) n->print(indent + 1); } 
    };


    // ----- rule -------------------------------------------------------------
    class rule {
    public:
        rule(char from, char to) {node_ = std::make_shared<literal>(from,to);}
        rule(char c) {node_ = std::make_shared<literal>(c);}
        rule(std::initializer_list<char> l) { 
            auto node=std::make_shared<or_node>();
            for (char c : l) 
                node->children_.push_back(std::make_shared<literal>(c));
            node_=node; 
        }
        rule(std::string s) {
            auto node=std::make_shared<next_node>();
            for (char c : s)
                node->children_.push_back(std::make_shared<literal>(c));
            node_=node;
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


    // ----- rule implementation ----------------------------------------------
    rule rule::operator| (const rule& l) { 
        return chain<or_node>(l);
    }

    rule rule::operator+ (const rule& l) { 
        return chain<next_node>(l);
    }

    rule rule::operator! () { 

        // Create not node.
        auto node = std::make_shared<not_node>();
        node->child = node_;

        // And a temp. parent rule.
        rule pnt;
        pnt.node_=node;
        return pnt;
    }

    template <class T>
    rule rule::chain(const rule &l) {
        rule pnt; // Create parent rule.
        pnt.node_=std::make_shared<T>();

        // Is it an or operator? If yes, just add to enable chaining.
        std::shared_ptr<T> this_node=td::dynamic_pointer_cast<T>(node_);
        if(this_node!=nullptr) 
            std::copy(
                this_node->children_.begin(), 
                this_node->children_.end(), 
                back_inserter(pnt.node_->children_));
        else 
            pnt.node_->children_.push_back(node_);

        pnt.node_->children_.push_back(l.node_);

        return pnt;
    }


} // namespace lingo

#endif // _LINGO_HPP