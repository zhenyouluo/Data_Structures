#ifndef TRIE_HPP
#define TRIE_HPP

#include <unordered_map>

struct TrieNode {
    explicit TrieNode(TrieNode* parent = nullptr) : parent(parent) {}

    /**
     * \brief Gets the child corresponding to the given character
     * \complexity Best: O(1) Worst: O(number of children)
     */
    TrieNode* get(char c) const {
        auto iter = children.find(c);
        if (iter == children.end())
            return nullptr;
        return iter->second;
    }

    /**
     * \brief Gets the child corresponding to the given character (creating it when needed)
     * \complexity Best: O(1) Worst: O(number of children)
     */
    TrieNode* get_or_create(char c) {
        if (children[c])
            return children[c];
        return children[c] = new TrieNode(this);
    }

    /**
     * \brief Copy the sub-tree rooted in the current node
     * \complexity O(n)
     */
    TrieNode* deep_copy() const {
        TrieNode* node = new TrieNode(nullptr);
        node->marks_end = marks_end;

        for (const auto& pair : children) {
            TrieNode* new_node = pair.second->deep_copy();
            new_node->parent = node;
            node->children[pair.first] = new_node;
        }

        return node;
    }

    TrieNode* parent = nullptr;
    bool marks_end = false;
    std::unordered_map<char,TrieNode*> children;
};


class Trie {
public:
    Trie() : root(new TrieNode) {}

    Trie(const Trie& other) : root(other.root->deep_copy()) {}

    Trie(Trie&& other) : root(other.root) { other.root = nullptr; }

    Trie& operator= (const Trie& other) {
        recursive_delete(root);
        root = other.root->deep_copy();
        return *this;
    }

    Trie& operator= (Trie&& other) {
        recursive_delete(root);
        root = other.root;
        other.root = nullptr;
        return *this;
    }

    ~Trie() {
        recursive_delete(root);
    }

    /**
     * \brief Add a new word to the trie
     * \complexity O(word.size)
     */
    void insert(const std::string& word) {
        TrieNode* node = root;
        for ( auto c : word )
            node = node->get_or_create(c);
        node->marks_end = true;
    }

    /**
     * \brief Remove a word from the trie
     * \complexity O(word.size)
     */
    void erase(const std::string& word) {
        TrieNode* node = root;
        for ( auto c : word ) {
            node = node->get(c);
            if (!node)
                return;
        }
        node->marks_end = false;
        remove_dangling(node);
    }

    /**
     * \brief Check if a word exists
     * \complexity O(word.size)
     */
    bool contains(const std::string& word) const {
        TrieNode* node = root;
        for ( auto c : word ) {
            node = node->get(c);
            if (!node)
                return false;
        }
        return node->marks_end;
    }

    /**
     * \brief Add a prefix exists
     * \complexity O(word.size)
     */
    bool contains_prefix(const std::string& word) const {
        TrieNode* node = root;
        for ( auto c : word ) {
            node = node->get(c);
            if (!node)
                return false;
        }
        return true;
    }

    /**
     * \brief Prints the tree tructure to stdout
     * \complexity O(n)
     */
    void print_structure() const {
        print_structure_recursive(root,"");
    }

private:
    /**
     * \brief Recursively removes a sub-tree
     * \complexity O(n)
     */
    void recursive_delete(TrieNode* node) {
        if (!node)
            return;
        for(const auto& child : node->children)
            recursive_delete(child.second);
        delete node;
    }

    /**
     * \brief Removes branches which don't lead to any word
     * \complexity O(h)
     */
    void remove_dangling(TrieNode* node) {
        while(node && node->parent != root &&
                !node->marks_end && node->children.size() <= 1)
            node = node->parent;
        if (node != root )
            recursive_delete(node);
    }

    /**
     * \brief Recursively prints a sub-tree
     * \complexity O(n)
     */
    void print_structure_recursive(const TrieNode* node, const std::string& prefix) const {
        if (node) {
            std::cout << prefix << (node->marks_end ? " *" : "") << '\n';
            for(const auto &pair : node->children) {
                print_structure_recursive(pair.second,prefix+pair.first);
            }
        }
    }

    TrieNode* root;
};

#endif // TRIE_HPP
