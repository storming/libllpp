#ifndef __LIBLLPP_RBTREE_H__
#define __LIBLLPP_RBTREE_H__

namespace ll {

class rbtree {
public:
    struct node {
        static constexpr unsigned red = 0;
        static constexpr unsigned black = 1;
        unsigned long _parent_color;
        node *_right;
        node *_left;

        node *parent() noexcept {
            return (node*)(_parent_color & ~3);
        }

        unsigned color() noexcept {
            return _parent_color & 1;
        }

        bool is_red() noexcept {
            return !(_parent_color & 1);
        }

        bool is_black() noexcept {
            return _parent_color & 1;
        }

        void set_red() noexcept {
            _parent_color &= ~1;
        }

        void set_black() noexcept {
            _parent_color |= 1;
        }

        void set_parent(node *parent) noexcept {
            _parent_color = (_parent_color & 3) | (unsigned long)parent;
        }

        void set_color(int color) noexcept {
            _parent_color = (_parent_color & ~1) | color;
        }

        node *next() noexcept {
            register node *parent, *node;

            if (this->parent() == this) {
                return nullptr;
            }

            /* If we have a right-hand child, go down and then left as far as we can. */
            if ((node = _right)) {
                while (node->_left) {
                    node = node->_left;
                }
                return node;
            }

            /* No right-hand children.  Everything down and left is
               smaller than us, so any 'next' node must be in the general
               direction of our parent. Go up the tree; any time the
               ancestor is a right-hand child of its parent, keep going
               up. First time it's a left-hand child of its parent, said
               parent is our 'next' node. */
            node = this;
            while ((parent = node->parent()) && node == parent->_right) {
                node = parent;
            }

            return parent;
        }

        node *prev() noexcept {
            register node *parent, *node;

            if (this->parent() == node) {
                return nullptr;
            }

            /* If we have a left-hand child, go down and then right as far as we can. */
            if ((node = _left)) {
                while (node->_right) {
                    node = node->_right;
                }
                return node;
            }

            /* No left-hand children. Go up till we find an ancestor which
               is a right-hand child of its parent */
            node = this;
            while ((parent = node->parent()) && node == parent->_left) {
                node = parent;
            }

            return parent;
        }
    };

protected:
    node *_root;

private:
    void rotate_left(register node *n) noexcept;
    void rotate_right(register node *n) noexcept;
    void erase_color(node *n, node *parent) noexcept;

public:
    rbtree() : _root() {}

    bool empty() noexcept {
        return !_root;
    }

    void init() noexcept {
        _root = nullptr;
    }

    node *root() noexcept {
        return _root;
    }

    node *front() noexcept {
        register node *node = _root;
        if (node) {
            while (node->_left) {
                node = node->_left;
            }
        }
        return node;
    }

    node *back() noexcept {
        register node *node = _root;
        if (node) {
            while (node->_right) {
                node = node->_right;
            }
        }
        return node;
    }

    static void link(node *n, node *parent, node **link) noexcept {
        n->_parent_color = (unsigned long )parent;
        n->_left = n->_right = nullptr;
        *link = n;
    }

    void insert(node *n) noexcept;
    void remove(node *n) noexcept;
    void replace(node *victim, node *new_node) noexcept;

};

}

#endif

