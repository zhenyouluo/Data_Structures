#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

/**
\file

\author Mattia Basaglia

\section License

Copyright (C) 2014  Mattia Basaglia

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <functional>
#include <iostream>
#include <type_traits>

template<class Key, class Value, class Comparator = std::less<Key>>
class RedBlackNode {
public:
    enum class Color {
        BLACK,
        RED,
    };

    RedBlackNode (const Key& key, const Value& value, Color color)
        : key(key), value(value), color(color) {}

    /**
     * \brief Finds a node containing the given key
     * \param search     The key to find
     * \param get_parent Whether to return the parent (for insertion)
     *                   rather than null on unmatched key
     */
    RedBlackNode* recursive_find(const Key& search, bool get_parent) {
        return noconst(&RedBlackNode::recursive_find, search, get_parent);
    }

    /**
     * \brief Finds a node containing the given key
     * \param search     The key to find
     * \param get_parent Whether to return the parent (for insertion)
     *                   rather than null on unmatched key
     */
    const RedBlackNode* recursive_find(const Key& search, bool get_parent) const {
        const RedBlackNode* fail_return = get_parent ? this : nullptr;
        if (Comparator()(search,key))
            return left ? left->recursive_find(search, get_parent) : fail_return;
        else if (Comparator()(key,search))
            return right ? right->recursive_find(search, get_parent) : fail_return;
        return this;
    }

    bool is_left_child() const {
        return parent && parent->left == this;
    }

    bool is_right_child() const {
        return parent && parent->right == this;
    }

    RedBlackNode* sibling() {
        return noconst(&RedBlackNode::sibling);
    }
    const RedBlackNode* sibling() const {
        if (!parent)
            return nullptr;
        return parent->left == this ? parent->right : parent->left;
    }

    /**
     * \brief Get node with minimum key in the current subtree
     * \complexity O(log n)
     */
    RedBlackNode* minimum() {
        return noconst(&RedBlackNode::minimum);
    }
    /**
     * \brief Get node with maximum key in the current subtree
     * \complexity O(log n)
     */
    RedBlackNode* maximum() {
        return noconst(&RedBlackNode::maximum);
    }

    /**
     * \brief Get successor (Node with the minimum key greater than this node's key)
     * \complexity O(log n)
     */
    RedBlackNode* successor() {
        return noconst(&RedBlackNode::successor);
    }

    /**
     * \brief Get predecessor (Node with the maximum key less than this node's key)
     * \complexity O(log n)
     */
    RedBlackNode* predecessor() {
        return noconst(&RedBlackNode::predecessor);
    }


    /**
     * \brief Get node with minimum key in the current subtree
     * \complexity O(log n)
     */
    const RedBlackNode* minimum() const {
        if (left)
            return left->minimum();
        return this;
    }
    /**
     * \brief Get node with maximum key in the current subtree
     * \complexity O(log n)
     */
    const RedBlackNode* maximum() const {
        if (right)
            return right->maximum();
        return this;
    }

    /**
     * \brief Get successor (Node with the minimum key greater than this node's key)
     * \complexity O(log n)
     */
    const RedBlackNode* successor() const {
        if (right)
            return right->minimum();
        const RedBlackNode* node = this;
        const RedBlackNode* p = parent;
        while(p && node->is_right_child()) {
            node = p;
            p = p->parent;
        }
        return p;
    }

    /**
     * \brief Get predecessor (Node with the maximum key less than this node's key)
     * \complexity O(log n)
     */
    const RedBlackNode* predecessor() const {
        if (left)
            return left->maximum();
        const RedBlackNode* node = *this;
        const RedBlackNode* p = parent;
        while(p && node->is_left_child()) {
            node = p;
            p = p->p;
        }
        return p;
    }

    /**
     * \brief Copy the sub-tree rooted in the current node
     * \complexity O(n)
     */
    RedBlackNode* deep_copy() const {
        RedBlackNode* node = new RedBlackNode(*this);
        if (left) {
            node->left = left->deep_copy();
            node->left->parent = node;
        }
        if (right) {
            node->right = right->deep_copy();
            node->right->parent = node;
        }
        node->parent = nullptr;

        return node;
    }

    Key   key;
    Value value;

    RedBlackNode *parent = nullptr;
    RedBlackNode *left   = nullptr;
    RedBlackNode *right  = nullptr;
    Color color;

private:
    /**
     * \brief Simple hack to avoid code duplication
     */
    template<class... FuncArgs, class... PassedArgs>
        RedBlackNode* noconst(const RedBlackNode* (RedBlackNode::*method)(FuncArgs...) const, PassedArgs... args) const {
            return const_cast<RedBlackNode*>((this->*method)(args...));
        }
};

template<class Key, class Value, class Comparator = std::less<Key>>
class RedBlackTree {
public:
    typedef const Key                               key_type;
    typedef Value                                   value_type;
    typedef RedBlackNode<key_type,Value,Comparator> node_type;
    typedef node_type*                              node_pointer;
    typedef const node_type*                        node_const_pointer;
    typedef typename node_type::Color               color_type;

    template<class NodePointer>
    class iterator_base {
    public:
        typedef NodePointer node_pointer;
        typedef decltype(NodePointer()->value) reference_type;
        typedef typename std::add_pointer<reference_type>::type pointer_type;
        typedef typename
            std::conditional<
                std::is_same<NodePointer,node_const_pointer>::value,
                const RedBlackTree*,
                RedBlackTree*
            >::type tree_pointer;

        iterator_base() : node(nullptr) {}

        iterator_base& operator++() {
            if (node) {
                node = node->successor();
            }
            return *this;
        }

        iterator_base& operator--() {
            if (node) {
                node = node->predecessor();
            } else if (tree->root_) {
                node = tree->root_->maximum();
            }
            return *this;
        }

        iterator_base operator++ (int) {
            iterator_base copy = *this;
            ++*this;
            return copy;
        }

        iterator_base operator-- (int) {
            iterator_base copy = *this;
            --*this;
            return copy;
        }

        reference_type operator* () const {
            return node->value;
        }

        pointer_type operator-> () const {
            return &node->value;
        }

        bool operator== (const iterator_base& other) const {
            return node == other.node;
        }

        bool operator!= (const iterator_base& other) const {
            return node != other.node;
        }

    private:
        iterator_base(tree_pointer tree, node_pointer node)
            : tree(tree), node(node) {}

        tree_pointer tree;
        node_pointer node;

        friend class RedBlackTree;
    };
    typedef iterator_base<node_pointer>         iterator;
    typedef iterator_base<node_const_pointer>   const_iterator;

    RedBlackTree() : root_(nullptr), size_(0) {}

    RedBlackTree (RedBlackTree&& other) : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    RedBlackTree (const RedBlackTree& other)
        : root_(other.root_->deep_copy()), size_(other.size_) {}

    RedBlackTree& operator= (const RedBlackTree& other) {
        recursive_delete(root_);
        root_ = other.root_->deep_copy();
        size_ = other.size_;
        return *this;
    }

    RedBlackTree& operator= (RedBlackTree&& other) {
        recursive_delete(root_);
        root_ = other.root_;
        size_ = other.size_;
        other.root_ = nullptr;
        other.size_ = 0;
        return *this;
    }

    ~RedBlackTree() {
        recursive_delete(root_);
    }

    /**
     * \brief Insert new node (assign if already present)
     * \complexity O(log n)
     */
    iterator insert (const Key& key, const Value& value) {
        return iterator(this, insert_node(key,value,true));
    }

    /**
     * \brief Remove a node
     * \complexity O(log n)
     */
    iterator erase (iterator it) {
        if (it == end() || it.tree != this)
            return end();
        iterator next = it;
        ++next;
        node_pointer todelete = erase_node(it.node);
        delete todelete;
        return next;
    }

    /**
     * \brief Remove a node
     * \complexity O(log n)
     */
    iterator erase(const Key &search) {
        return erase(find(search));
    }

    /**
     * \brief Get reference to given value (inserted with default-constructed value if not present)
     * \complexity O(log n)
     */
    value_type& operator[] (const Key& key) {
        node_pointer p = insert_node(key,Value(),false);
        return p->value;
    }

    /**
     * \brief Print the tree tructure to stdout
     * \complexity O(n)
     */
    void print_structure() const {
        print_structure_recursive(root_,0);
    }

    /**
     * \brief Get iterator to the first element
     * \complexity O(log n)
     */
    iterator begin() {
        return iterator(this,root_ ? root_->minimum() : nullptr);
    }
    /**
     * \brief Get iterator to past-the-last element
     * \complexity O(1)
     */
    iterator end() {
        return iterator(this, nullptr);
    }

    /**
     * \brief Get iterator to the first element
     * \complexity O(log n)
     */
    const_iterator begin() const {
        return const_iterator(this,root_ ? root_->minimum() : nullptr);
    }
    /**
     * \brief Get iterator to past-the-last element
     * \complexity O(1)
     */
    const_iterator end() const {
        return const_iterator(this, nullptr);
    }

    /**
     * \brief Get iterator to the first element
     * \complexity O(log n)
     */
    const_iterator cbegin() const {
        return begin();
    }
    /**
     * \brief Get iterator to past-the-last element
     * \complexity O(1)
     */
    const_iterator cend() const {
        return end();
    }


    /**
     * \brief Get iterator to the given key
     * \complexity O(log n)
     */
    iterator find(const Key& search) {
        if (empty())
            return end();
        return iterator(this, root_->recursive_find(search,false));
    }

    /**
     * \brief Get iterator to the given key
     * \complexity O(log n)
     */
    const_iterator find(const Key& search) const {
        if (empty())
            return end();
        return const_iterator(this, root_->recursive_find(search,false));
    }

    /**
     * \brief Number of nodes in the tree
     * \complexity O(1)
     */
    int size() const {
        return size_;
    }

    /**
     * \brief Whether the tree is empty
     */
    bool empty() const {
        return !root_;
    }


    template<class Policy, class Func>
        void traverse(const Func& func)
        {
            Policy::traverse(root_, func);
        }

protected:
    /**
     * \brief Left rotation
     * \pre \c node and \c node->left aren't null
     * \complexity O(1)
     */
    void rotate_left(node_pointer node) {
        // get next root of the subtree currently rooted in node
        node_pointer next_subroot = node->right;
        // change the middle subtree
        node->right = next_subroot->left;
        if (node->right)
            node->right->parent = node;
        // fix parent
        next_subroot->parent = node->parent;
        if (!node->parent)
            root_ = next_subroot;
        else if (node->is_left_child())
            node->parent->left = next_subroot;
        else
            node->parent->right = next_subroot;
        // set the old root ad left child
        next_subroot->left = node;
        node->parent = next_subroot;
    }

    /**
     * \brief Right rotation
     * \pre \c node and \c node->right aren't null
     * \complexity O(1)
     */
    void rotate_right (node_pointer node) {
        // get next root of the subtree currently rooted in node
        node_pointer next_subroot = node->left;
        // change the middle subtree
        node->left = next_subroot->right;
        if (node->left)
            node->left->parent = node;
        // fix parent
        next_subroot->parent = node->parent;
        if (!node->parent)
            root_ = next_subroot;
        else if (node->is_left_child())
            node->parent->left = next_subroot;
        else
            node->parent->right = next_subroot;
        // set the old root as right child
        next_subroot->right = node;
        node->parent = next_subroot;
    }

    /**
     * \brief Recursively deletes a sub-tree
     * \note Doesn't fix parent pointers
     */
    void recursive_delete(node_pointer node) {
        if (node->left)
            recursive_delete(node->left);
        if (node->right)
            recursive_delete(node->right);
        delete node;
    }

    /**
     * \brief Fix color of a newly inserted node
     * \complexity O(log n)
     */
    void insert_fixup(node_pointer node) {
        while (node && node->parent && node->parent->color == color_type::RED) {
            if (node->parent->is_left_child()) {
                node_pointer y = node->parent->parent->right;
                if (y && y->color == color_type::RED) {
                    node->parent->color = color_type::BLACK;
                    y->color = color_type::BLACK;
                    node->parent->parent->color = color_type::RED;
                    node = node->parent->parent;
                } else {
                    if (node->is_right_child()) {
                        node = node->parent;
                        rotate_left(node);
                    }
                    node->parent->color = color_type::BLACK;
                    node->parent->parent->color = color_type::RED;
                    rotate_right(node->parent->parent);
                }
            } else {
                node_pointer y = node->parent->parent->left;
                if (y && y->color == color_type::RED) {
                    node->parent->color = color_type::BLACK;
                    y->color = color_type::BLACK;
                    node->parent->parent->color = color_type::RED;
                    node = node->parent->parent;
                } else {
                    if (node->is_left_child()) {
                        node = node->parent;
                        rotate_right(node);
                    }
                    node->parent->color = color_type::BLACK;
                    node->parent->parent->color = color_type::RED;
                    rotate_left(node->parent->parent);
                }
            }
        }
        root_->color = color_type::BLACK;
    }

    /**
     * \brief Insert new node (assign if already present)
     * \complexity O(log n)
     * \return Matching node
     */
    node_pointer insert_node(const Key& key, const Value& value, bool assign) {
        if (!root_)
            return root_ = new node_type(key,value,color_type::BLACK);
        node_pointer location = root_->recursive_find(key,true);
        if (Comparator()(key,location->key)) {
            node_pointer new_node = new node_type(key,value,color_type::RED);
            location->left = new_node;
            new_node->parent = location;
            insert_fixup(new_node);
            size_++;
            return new_node;
        } else if (Comparator()(location->key,key)) {
            node_pointer new_node = new node_type(key,value,color_type::RED);
            location->right = new_node;
            new_node->parent = location;
            insert_fixup(new_node);
            size_++;
            return new_node;
        } else if (assign) {
            // assign already existing node
            location->value = value;
        }
        return location;
    }

    /**
     * \brief Fix color caused by node deletion
     * \complexity O(log n)
     */
    void erase_fixup(node_pointer node) {
        while (node != root_ && node->color == color_type::BLACK) {

            node_pointer sib = node->sibling();
            if (sib && sib->color == color_type::RED) {
                sib->color = color_type::BLACK;
                node->parent->color = color_type::RED;
                if (node->is_left_child())
                    rotate_left(node->parent);
                else
                    rotate_right(node->parent);
                sib = node->sibling();
            }
            if (sib->left->color == color_type::BLACK && sib->right->color == color_type::BLACK) {
                sib->color = color_type::RED;
                node = node->parent;
            } else if (node->is_left_child()) {
                if (sib->right->color == color_type::BLACK) {
                    sib->left->color = color_type::BLACK;
                    sib->color = color_type::RED;
                    rotate_right(sib);
                    sib = node->sibling();
                }
                sib->color = node->parent->color;
                node->parent->color = color_type::RED;
                sib->right->color = color_type::BLACK;
                rotate_left(node->parent);
                node = root_;
            } else {
                // same as above, swapping left and right
                if (sib->left->color == color_type::BLACK) {
                    sib->right->color = color_type::BLACK;
                    sib->color = color_type::RED;
                    rotate_left(sib);
                    sib = node->sibling();
                }
                sib->color = node->parent->color;
                node->parent->color = color_type::RED;
                sib->left->color = color_type::BLACK;
                rotate_right(node->parent);
                node = root_;

            }

        }
        node->color = color_type::BLACK;
    }

    /**
     * \brief Remove an existing node
     * \complexity O(log n) (successor() call)
     * \return Node to be deleted
     */
    node_pointer erase_node (node_pointer node) {
        // y is the node to be removed (has at most 1 child)
        node_pointer y = !node->left || !node->right ? node : node->successor();
        // x is the only child of y (if it has 1 child) null otherwise
        node_pointer x = y->left ? y->left : y->right;

        if (x)
            x->parent = y->parent;

        if (!y->parent) {
            root_ = x;
        } else if (y->is_left_child()) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }

        if (y != node) {
            // should remove y, replace node instead
            if ((y->parent = node->parent)) {
                if (node->is_left_child())
                    y->parent->left = y;
                else
                    y->parent->right = y;
            } else {
                root_ = y;
            }
            if ((y->left = node->left)) {
                y->left->parent = y;
            }
            if ((y->right = node->right))  {
                y->right->parent = y;
            }
        }

        if (y->color == color_type::BLACK)
            erase_fixup(x);

        size_--;
        return node;
    }

private:
    node_pointer root_;
    int size_;

    void print_structure_recursive(node_const_pointer node, int depth) const {
        if (node) {
            print_structure_recursive(node->left,depth+1);
            std::string color;
            std::string nocolor = "\x1b[0m";
            if (node->color == color_type::RED)
                color = "\x1b[31m";
            std::cout << std::string(depth,'.') << " "
                      << color << "(" << nocolor << node->key << color << ")" << nocolor << " "
                      << node->value << '\n';
            print_structure_recursive(node->right,depth+1);
        } else {
            std::cout << std::string(depth,'.') << " NULL\n";
        }
    }
};

namespace traversal {

struct Forward
{
    template<class NodePtr>
        static NodePtr first(NodePtr node)
        {
            return node->left;
        }

    template<class NodePtr>
        static NodePtr last(NodePtr node)
        {
            return node->right;
        }
};

struct Reverse
{
    template<class NodePtr>
        static NodePtr first(NodePtr node)
        {
            return node->right;
        }

    template<class NodePtr>
        static NodePtr last(NodePtr node)
        {
            return node->left;
        }
};

template<class Direction = Forward>
    struct InOrder
{
    template<class NodePtr, class Func>
        static void traverse(NodePtr node, const Func& func)
        {
            if ( !node )
                return;
            traverse(Direction::first(node), func);
            func(node->key, node->value);
            traverse(Direction::last(node), func);
        }
};

template<class Direction = Forward>
    struct PreOrder
{
    template<class NodePtr, class Func>
        static void traverse(NodePtr node, const Func& func)
        {
            if ( !node )
                return;
            func(node->key, node->value);
            traverse(Direction::first(node), func);
            traverse(Direction::last(node), func);
        }
};

template<class Direction = Forward>
    struct PostOrder
{
    template<class NodePtr, class Func>
        static void traverse(NodePtr node, const Func& func)
        {
            if ( !node )
                return;
            traverse(Direction::first(node), func);
            traverse(Direction::last(node), func);
            func(node->key, node->value);
        }
};


} // namespace traversal

#endif // RED_BLACK_TREE_HPP
