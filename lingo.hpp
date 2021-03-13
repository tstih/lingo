#ifndef _LINGO_HPP
#define _LINGO_HPP

#include <vector>
#include <memory>
#include <sstream>
#include <algorithm>
#include <atomic>
#include <stack>

namespace lingo {


    // ----- parse exception --------------------------------------------------
    class parse_exception : public std::exception {
    public:
        enum class exception_type { unexpected };
        parse_exception(
            std::string source_name,
            std::string rule_name,
            int row,
            int col) :
                source_name_(source_name), 
                rule_name_(rule_name), 
                row_(row), 
                col_(col)
            {};
        // TODO: Expose members.
    protected:
        std::string source_name_;
        std::string rule_name_;
        int row_;
        int col_;
    };


    // ----- source (code) reader  --------------------------------------------
    class source {
    public:
        virtual std::string name() const = 0; // Return current source name.
        virtual int row() const = 0; // Return current row.
        virtual int col() const = 0; // Return current column.

        virtual char peek() const = 0; // Peek at next char.
        virtual char consume() = 0; // Consume next char (mark as read!)

        virtual int bookmark() = 0; // Remember current position (and return it). 
        virtual void bookmark(int bookmark) = 0; // Go to bookmark!
    };

    class string_source : public source {
    public:
        string_source(std::string code) : code_(code), pos_(0) {}
        std::string name() const override { return std::string(); }
        int row() const override { return row_; }
        int col() const override { return col_;}
        char peek() const override { return code_[pos_]; };
        char consume() override {return code_[pos_++];}
        int bookmark() override { return pos_; }
        void bookmark(int p) override { pos_=p;}
    private:
        std::string code_;
        int pos_, row_, col_;
    };


    // ----- forward defintions  ----------------------------------------------
    class rule;
    class node;
    class literal;
    class and_node;
    class or_node;
    class not_node;
    class repeat_node;
    class placeholder_node;


    // ----- nodes ------------------------------------------------------------
    class node_visitor {
    public:
        virtual void visit_literal(literal &element) {}
        virtual void visit_and_node(and_node &element) {}
        virtual void visit_or_node(or_node &element) {}
        virtual void visit_not_node(not_node &element) {}
        virtual void visit_repeat_node(repeat_node &element) {}
        virtual void visit_placeholder_node(placeholder_node &element) {}
    };

    class node {
    public:
        node() { id_ = ++next_id_; name_=internal_name(); }
        node(std::string name) : name_(name) { id_ = ++next_id_;}
        virtual bool parse(source& src)=0;
        virtual void accept(node_visitor &visitor) {} // Default implementation.
        int id() const { return id_; };
        virtual std::string name() { return name_; }
        void name(std::string name) { name_=name; }
    protected:
        // Node name. Optional. Can be empty.
        std::string name_;
        // Internal node identifier. This is also used for name, when no name is provided.
        int id_;
        static std::atomic<int> next_id_;
        std::string internal_name() { std::stringstream ss; ss<<id(); return ss.str();}
    };
    // Static definition.
    std::atomic<int> node::next_id_ = {0};

    class unary_node : public node { // Just 1 child.
    public:
        unary_node() : node() {}
        unary_node(std::string name) : node(name) {}
        virtual bool parse(source& src)=0;
        std::shared_ptr<node> sibling() { return sibling_; }
        void sibling(std::shared_ptr<node> sibling) { sibling_=sibling; }
    protected:
        std::shared_ptr<node> sibling_;
    };

    class multary_node : public node { // N children.
    public:
        multary_node() : node() {}
        multary_node(std::string name) : node(name) {}
        virtual bool parse(source& src)=0;
        std::vector<std::shared_ptr<node>>& children() { return children_; }
    protected:
        std::vector<std::shared_ptr<node>> children_;
    };

    class literal : public node { // Special node, without children.
    public:
        literal(char c) : node(), from_(c), to_(c) {}
        literal(std::string name, char c) : node(name), from_(c), to_(c) {}
        literal(char from, char to) : node(), from_(from), to_(to) {}
        literal(std::string name, char from, char to) : node(name), from_(from), to_(to) {}
        virtual bool parse(source& src) {
            char symbol=src.consume();
            return (symbol>=from_ && symbol<=to_);   
        }
        void accept(node_visitor &visitor) override {
            visitor.visit_literal(*this);
        } 
        char from() { return from_; }
        char to() { return to_; }
    private:
        char from_;
        char to_;
    };

    class or_node : public multary_node {
    public:
        or_node() : multary_node() {}
        or_node(std::string name) : multary_node(name) {}
        virtual bool parse(source& src) {
            auto bm=src.bookmark(); // Remember pos.
            for(auto n:children_) {
                bool result = n->parse(src);
                if (!result) 
                    src.bookmark(bm); // Back to pos.
                else 
                    return true; // Just need one for or.
            }
            return false;
        }
        void accept(node_visitor &visitor) override {
            visitor.visit_or_node(*this);
        } 
    };

    class and_node : public multary_node {
    public:
        and_node() : multary_node() {}
        and_node(std::string name) : multary_node(name) {}
        virtual bool parse(source& src) {
            auto bm=src.bookmark(); // Remember pos.
            for(auto n:children_) {
                bool result = n->parse(src);
                if (!result) 
                    return false;
            }
            return true; // We did it!
        }
        void accept(node_visitor &visitor) override {
            visitor.visit_and_node(*this);
        } 
    };

    class not_node : public unary_node {
    public:
        not_node() : unary_node() {}
        not_node(std::string name) : unary_node(name) {}
        virtual bool parse(source& src) {
            auto bm=src.bookmark(); // Remember pos.
            bool result = sibling_->parse(src);
            return !result;
        }
        void accept(node_visitor &visitor) override {
            visitor.visit_not_node(*this);
        } 
    };

    class repeat_node : public unary_node {
    public:
        repeat_node(int min=1, int max=0) : unary_node(), min_(min), max_(max) {}
        repeat_node(std::string name, int min=1, int max=0) : unary_node(name), min_(min), max_(max) {}
        virtual bool parse(source& src) {
            int n=0;
            while (sibling_->parse(src) && (max_==0 || n<=max_)) n++;
            return (n>=min_ && (max_==0 || n<=max_));
        }
        void accept(node_visitor &visitor) override {
            visitor.visit_repeat_node(*this);
        } 
        int min() const { return min_; }
        int max() const { return max_; }
    private:
        friend rule repeat(const rule& r, int min , int max);
        int min_;
        int max_;
    };

    class placeholder_node : public unary_node {
    public:
        placeholder_node() : unary_node() {}
        placeholder_node(std::string name) : unary_node(name) {}
        void accept(node_visitor &visitor) override {
            visitor.visit_placeholder_node(*this);
        } 
        virtual bool parse(source& src) {
            return true;
        }
        void set(std::shared_ptr<node> sibling) { sibling_=sibling; }
    private:
        friend rule placeholder();
    };


    // ----- ast_node ---------------------------------------------------------
    class ast_node { // The node.
    public:
        
    };


    class ast { // The tree.

    };

    // ----- rule -------------------------------------------------------------
    class rule {
    public:
        rule(const rule & r) { node_=r.node_; }
        rule(std::string name, const rule & r) { node_=r.node_; node_->name(name);}
        rule(char from, char to) {node_ = std::make_shared<literal>(from,to);}
        rule(std::string name, char from, char to) {node_ = std::make_shared<literal>(name,from,to);}
        rule(char c) {node_ = std::make_shared<literal>(c);}
        rule(std::string name, char c) {node_ = std::make_shared<literal>(name,c);}
        rule(std::initializer_list<char> l) { 
            auto node=std::make_shared<or_node>();
            for (char c : l) 
                node->children().push_back(std::make_shared<literal>(c));
            node_=node; 
        }
        rule(std::string name, std::initializer_list<char> l) { 
            auto node=std::make_shared<or_node>(name);
            for (char c : l) 
                node->children().push_back(std::make_shared<literal>(c));
            node_=node; 
        }
        rule(std::string s) {
            auto node=std::make_shared<and_node>();
            for (char c : s)
                node->children().push_back(std::make_shared<literal>(c));
            node_=node;
        }
        rule(std::string name, std::string s) {
            auto node=std::make_shared<and_node>(name);
            for (char c : s)
                node->children().push_back(std::make_shared<literal>(c));
            node_=node;
        }

        virtual ~rule()=default;

        // Or, and, not rules.
        rule operator| (const rule& r);
        rule operator+ (const rule& r);
        rule operator! ();

        // Node visitor.
        void accept(node_visitor & v) { node_->accept(v); }

        bool validate(source& src) { return node_->parse(src); }

    protected:
        template <class T> rule multary(const rule &r);
        template <class T> rule unary();

    private:
        friend rule repeat(const rule& r, int min , int max);
        friend class placeholder;
        rule() {} // Empty rule ctor.
        std::shared_ptr<node> node_;
    };


    class placeholder : public rule {
    public:
        placeholder() : rule() {
            node_=std::make_shared<placeholder_node>();
        }

        void set(const rule& r) {

            std::shared_ptr<placeholder_node> this_node=
                std::dynamic_pointer_cast<placeholder_node>(node_);

            this_node->set(r.node_);
        }
    };

    // ----- rule implementation ----------------------------------------------
    rule rule::operator| (const rule& r) { 
        return multary<or_node>(r);
    }

    rule rule::operator+ (const rule& r) { 
        return multary<and_node>(r);
    }

    rule rule::operator! () { 
        return unary<not_node>();
    }

    template <class T>
    rule rule::unary() {
        // Create unary node.
        std::shared_ptr<unary_node> node = std::make_shared<T>();
        node->sibling(node_);

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
                this_node->children().begin(), 
                this_node->children().end(), 
                back_inserter(node->children()));
        } else 
            node->children().push_back(node_);

        // Add rule tree.
        node->children().push_back(r.node_);

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


    // ----- visitors ---------------------------------------------------------
#ifdef DIAGNOSTICS
    class graphviz_export_node_visitor : public node_visitor {
    public:
        graphviz_export_node_visitor() : count_(0){ }
        void visit_literal(literal &n) {
            std::string me=name(n);
            o_ << n.id() << " [label=" << me << "]" << std::endl;
            if (!parents_.empty()) o_ << parents_.top() << "->" << n.id() << std::endl;
        }
        void visit_and_node(and_node &n) {
            visit_multary(n);
        }
        void visit_or_node(or_node &n) {
            visit_multary(n);
        }
        void visit_not_node(not_node &n) {
            visit_unary(n);
        }
        void visit_repeat_node(repeat_node &n) {
            visit_unary(n);
        }
        void visit_placeholder_node(placeholder_node &n) {
            visit_unary(n);
        }
        std::string str() {
            std::stringstream result; 
            result << "digraph G {" << std::endl;
            result << "node [fontname=\"Arial\", shape=plaintext];" << std::endl;
            result << "ordering=out" << std::endl;
            result << o_.str();
            result << "}" << std::endl;
            return result.str();
        }
    private:

        std::string name(literal &n) {
            std::stringstream ss;
            if (n.from()!=n.to())
                ss << "\"{" << n.from() << "-" << n.to() << "}\"";
            else
                ss << "\"{" << n.from() << "}\"";
            return ss.str();
        }

        std::string name(and_node &n) { return "<and>"; }
        std::string name(or_node &n) { return "<or>"; }
        std::string name(not_node &n) { return "<not>"; }
        std::string name(placeholder_node &n) { return "<placeholder>"; }

        std::string name(repeat_node &n) {
            std::stringstream ss;
            ss << "\"" << "repeat (";
            if (n.min()==0) ss<<"nil"; else ss<<n.min();
            ss<<",";
            if (n.max()==0) ss<<"infinite"; else ss<<n.max();
            ss<< ")" << "\"";
            return ss.str();
        }

        template<class T>
        void visit_multary(T &n) {
            // Conjure element name and push onto parents stack.
            std::string me=name(n);
            o_ << n.id() << " [label=" << me << "]"<< std::endl;
            if (!parents_.empty()) o_ << parents_.top() << "->" << n.id() << std::endl;

            // Iterate children.
            if(!visited(&n)) { // Never visited before?
                audit(&n); // Add to visited nodes (prevent recursion).
                parents_.push(n.id()); // Parent to stack.
                for(auto c:n.children()) c->accept(*this);
                parents_.pop();
            }
        }

        template<class T>
        void visit_unary(T &n) {
            // Conjure element name and push onto parents stack.
            std::string me=name(n);
            o_ << n.id() << " [label=" << me << "]"<< std::endl;
            if (!parents_.empty()) o_ << parents_.top() << "->" << n.id() << std::endl;

            // Iterate children.
            if(!visited(&n)) { // Never visited before?
                audit(&n); // Audit our visit.
                parents_.push(n.id()); // Parent to stack.
                n.sibling()->accept(*this);
                parents_.pop();
            }
        }

        // Output.
        std::stringstream o_;
        // Parent stack.
        std::stack<int> parents_;
        // Prevent cycles.
        void audit(node *n) {
            visited_.push_back(n);
        }
        bool visited(node *n) {
            return (std::find(visited_.begin(), visited_.end(), n) != visited_.end());
        }
        std::vector<node *> visited_;
        int count_;
    };
#endif // DIAGNOSTICS

} // namespace lingo

#endif // _LINGO_HPP