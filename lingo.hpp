#ifndef _LINGO_HPP
#define _LINGO_HPP

#include <vector>
#include <memory>
#include <iostream>

namespace lingo {


    // ----- source (code) reader  --------------------------------------------
    class source {
    public:
        source(std::string code) : code_(code), pos_(0) {}
        int bookmark() { return pos_; }
        void bookmark(int p) { pos_=p;}
        char peek() { return code_[pos_]; };
        char consume() {return code_[pos_++];}
    private:
        std::string code_;
        int pos_;
    };


    // ----- forward defintions  ----------------------------------------------
    class rule;


    // ----- nodes ------------------------------------------------------------
    class node {
    public:
        virtual void print(int indent=0)=0;
        virtual bool parse(source& src)=0;
    };

    class unary_node : public node { // Just 1 child.
    public:
        virtual void print(int indent)=0;
        virtual bool parse(source& src)=0;
    protected:
        friend class rule;
        std::shared_ptr<node> sibling_;
    };

    class multary_node : public node { // N children.
    public:
        virtual void print(int indent)=0;
        virtual bool parse(source& src)=0;
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
        virtual bool parse(source& src) {
            char symbol=src.consume();
            return (symbol>=from_ && symbol<=to_);   
        }
    private:
        char from_;
        char to_;
    };

    class or_node : public multary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "or" << std::endl; for(auto n:children_) n->print(indent + 1); }
        virtual bool parse(source& src) {
            auto bm=src.bookmark(); // Remember pos.
            for(auto n:children_) {
                bool result = n->parse(src);
                if (!result) 
                    src.bookmark(bm); // Back to pos.
                else 
                    return true; // Just need one for or.
            }
        }
    };

    class next_node : public multary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "next" << std::endl; for(auto n:children_) n->print(indent + 1); }
        virtual bool parse(source& src) {
            auto bm=src.bookmark(); // Remember pos.
            for(auto n:children_) {
                bool result = n->parse(src);
                if (!result) 
                    return false;
            }
            return true; // We did it!
        }
    };

    class not_node : public unary_node {
    public:
        void print(int indent) override { std::cout << std::string(indent, ' ') << "not" << std::endl; sibling_->print(indent + 1); }    
        virtual bool parse(source& src) {
            auto bm=src.bookmark(); // Remember pos.
            bool result = sibling_->parse(src);
            return !result;
        }
    };

    class repeat_node : public unary_node {
    public:
        repeat_node(int min=1, int max=0) : min_(min), max_(max) {}
        void print(int indent) override { std::cout << std::string(indent, ' ') << "repeat (" << min_ << "," << max_ << ")" << std::endl; sibling_->print(indent + 1); } 
        virtual bool parse(source& src) {
            int n=0;
            while (sibling_->parse(src) && (max_==0 || n<=max_)) n++;
            return (n>=min_ && (max_==0 || n<=max_));
        }
    private:
        friend rule repeat(const rule& r, int min , int max);
        int min_;
        int max_;
    };

    class placeholder_node : public unary_node {
    public:
        void print(int indent) override { 
            std::cout << std::string(indent, ' ') << "placeholder" << std::endl; 
            if (sibling_==nullptr)
                std::cout << std::string(indent + 1, ' ') << "(empty)" << std::endl; 
            else 
                sibling_->print(indent + 1);
        } 
        virtual bool parse(source& src) {
            return true;
        }
        void set(std::shared_ptr<node> sibling) { sibling_=sibling; }
    private:
        friend rule placeholder();
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

        rule operator| (const rule& r);
        rule operator+ (const rule& r);
        rule operator! ();

        void print() { node_->print(); }
        bool parse(source& src) { return node_->parse(src); }

    protected:
        template <class T> rule multary(const rule &r);
        template <class T> rule unary();

    private:
        rule() {} // Empty rule ctor.
        friend rule repeat(const rule& r, int min , int max);
        friend rule placeholder();
        std::shared_ptr<node> node_;
    };


    // ----- rule implementation ----------------------------------------------
    rule rule::operator| (const rule& r) { 
        return multary<or_node>(r);
    }

    rule rule::operator+ (const rule& r) { 
        return multary<next_node>(r);
    }

    rule rule::operator! () { 
        return unary<not_node>();
    }

    template <class T>
    rule rule::unary() {
        // Create unary node.
        std::shared_ptr<unary_node> node = std::make_shared<T>();
        node->sibling_ = node_;

        // And a temp. parent rule.
        rule pnt;
        pnt.node_=node;
        return pnt;
    }

    template <class T>
    rule rule::multary(const rule &r) {

        // This will be our result.
        std::shared_ptr<multary_node> node=std::make_shared<T>();

        // Is it the same operator? If yes, enable chaining.
        if(std::dynamic_pointer_cast<T>(node_)!=nullptr)  {
            // Get this node as multary node.
            std::shared_ptr<multary_node> this_node=
                std::dynamic_pointer_cast<multary_node>(node_);
            // Copy existing items to new node.
            std::copy(
                this_node->children_.begin(), 
                this_node->children_.end(), 
                back_inserter(node->children_));
        } else 
            node->children_.push_back(node_);

        // Add rule tree.
        node->children_.push_back(r.node_);

        // And return new rule.
        rule pnt;
        pnt.node_=node;
        return pnt;
    }


    // ----- global functions -------------------------------------------------
    rule repeat(const rule& r, int min = 1, int max = 0) {
        // Create unary node.
        auto node = std::make_shared<repeat_node>(min,max);
        node->sibling_ = r.node_;

        // And a temp. parent rule.
        rule pnt;
        pnt.node_=node;
        return pnt;
    }

    rule placeholder() {
        // Create unary node without sibling.
        auto node = std::make_shared<placeholder_node>();
        node->sibling_=nullptr;
        // And a temp. parent rule.
        rule pnt;
        pnt.node_=node;
        return pnt;
    }

} // namespace lingo

#endif // _LINGO_HPP