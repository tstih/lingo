#ifndef _NODE_HPP
#define _NODE_HPP

#include <vector>
#include <memory>
#include <iostream>

namespace lingo {

    class node {
    public:
        virtual void print(int indent=0)=0;
    };

    class parent_node : public node {
    public:
        virtual void print(int indent)=0;
    protected:
        friend class rule;
        std::vector<std::shared_ptr<node>> children_;
    };

    class literal : public node {
    public:
        literal(char c) : from_(c), to_(c) {}
        literal(char from, char to) : from_(from), to_(to) {}
        void print(int indent) override { std::cout << std::string(indent, ' ') << "literal (" << from_ << "," << to_ << ")" <<  std::endl; }
    private:
        char from_;
        char to_;
    };

    class or_node : public parent_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "or" << std::endl; for(auto n:children_) n->print(indent + 1); }
    };

    class next_node : public parent_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "next" << std::endl; for(auto n:children_) n->print(indent + 1); }
    };

    class not_node : public node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "not" << std::endl; for(auto n:children_) n->print(indent + 1); }    
    };

    class repeat_node : public node {
    public:
        repeat_node(int min=1, int max=0) : min_(min), max_(max) {}
        void print(int indent) override { std::cout << std::string(indent, ' ') << "repeat (" << min_ << "," << max_ << ")" << std::endl; for(auto n:children_) n->print(indent + 1); } 
    private:
        int min_;
        int max_;
    };

    class optional_node : public node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "optional" << std::endl; for(auto n:children_) n->print(indent + 1); } 
    };
}

#endif // _NODE_HPP