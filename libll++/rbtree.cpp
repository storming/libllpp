#include "rbtree.h"


namespace ll {

inline void rbtree::rotate_left(register rbtree::node *n) 
{
    register node *right = n->_right;
    register node *parent = n->parent();

    if ((n->_right = right->_left)) {
        right->_left->set_parent(n);
    }

    right->_left = n;
    right->set_parent(parent);

    if (parent) {
        if (n == parent->_left) {
            parent->_left = right;
        }
        else {
            parent->_right = right;
        }
    }
    else {
        _root = right;
    }

    n->set_parent(right);
}

inline void rbtree::rotate_right(register node *n)
{
    register node *left = n->_left;
    register node *parent = n->parent();

    if ((n->_left = left->_right)) {
        left->_right->set_parent(n);
    }
    left->_right = n;

    left->set_parent(parent);

    if (parent) {
        if (n == parent->_right) {
            parent->_right = left;
        }
        else {
            parent->_left = left;
        }
    }
    else {
        _root = left;
    }
    n->set_parent(left);
}

void rbtree::insert(node *n)
{
    node *parent, *gparent;

    while ((parent = n->parent()) && parent->is_red()) {
        gparent = parent->parent();

        if (parent == gparent->_left) {
            {
                register node *uncle = gparent->_right;
                if (uncle && uncle->is_red()) {
                    uncle->set_black();
                    parent->set_black();
                    gparent->set_red();
                    n = gparent;
                    continue;
                }
            }

            if (parent->_right == n) {
                register node *tmp;
                rotate_left(parent);
                tmp = parent;
                parent = n;
                n = tmp;
            }

            parent->set_black();
            gparent->set_red();
            rotate_right(gparent);
        }
        else {
            {
                register node *uncle = gparent->_left;
                if (uncle && uncle->is_red()) {
                    uncle->set_black();
                    parent->set_black();
                    gparent->set_red();
                    n = gparent;
                    continue;
                }
            }

            if (parent->_left == n) {
                register node *tmp;
                rotate_right(parent);
                tmp = parent;
                parent = n;
                n = tmp;
            }

            parent->set_black();
            gparent->set_red();
            rotate_left(gparent);
        }
    }

    _root->set_black();
}

inline void rbtree::erase_color(node *n, node *parent)
{
    node *other;

    while ((!n || n->is_black()) && n != _root) {
        if (parent->_left == n) {
            other = parent->_right;
            if (other->is_red()) {
                other->set_black();
                parent->set_red();
                rotate_left(parent);
                other = parent->_right;
            }
            if ((!other->_left || other->_left->is_black()) &&
                (!other->_right || other->_right->is_black())) {
                other->set_red();
                n = parent;
                parent = n->parent();
            }
            else {
                if (!other->_right || other->_right->is_black()) {
                    node *o_left;
                    if ((o_left = other->_left)) {
                        o_left->set_black();
                    }
                    other->set_red();
                    rotate_right(other);
                    other = parent->_right;
                }
                other->set_color(parent->color());
                parent->set_black();
                if (other->_right) {
                    other->_right->set_black();
                }
                rotate_left(parent);
                n = _root;
                break;
            }
        }
        else {
            other = parent->_left;
            if (other->is_red()) {
                other->set_black();
                parent->set_red();
                rotate_right(parent);
                other = parent->_left;
            }
            if ((!other->_left || other->_left->is_black()) &&
                (!other->_right || other->_right->is_black())) {
                other->set_red();
                n = parent;
                parent = n->parent();
            }
            else {
                if (!other->_left || other->_left->is_black()) {
                    register node *o_right;
                    if ((o_right = other->_right)) {
                        o_right->set_black();
                    }
                    other->set_red();
                    rotate_left(other);
                    other = parent->_left;
                }
                other->set_color(parent->color());
                parent->set_black();
                if (other->_left) {
                    other->_left->set_black();
                }
                rotate_right(parent);
                n = _root;
                break;
            }
        }
    }
    if (n) {
        n->set_black();
    }
}

void rbtree::remove(node *n) 
{
    node *child, *parent;
    unsigned color;

    if (!n->_left) {
        child = n->_right;
    }
    else if (!n->_right) {
        child = n->_left;
    }
    else {
        node *old = n, *left;

        n = n->_right;
        while ((left = n->_left) != 0) {
            n = left;
        }
        child = n->_right;
        parent = n->parent();
        color = n->color();

        if (child) {
            child->set_parent(parent);
        }
        if (parent == old) {
            parent->_right = child;
            parent = n;
        }
        else {
            parent->_left = child;
        }

        n->_parent_color = old->_parent_color;
        n->_right = old->_right;
        n->_left = old->_left;

        if (old->parent()) {
            if (old->parent()->_left == old) {
                old->parent()->_left = n;
            }
            else {
                old->parent()->_right = n;
            }
        }
        else {
            _root = n;
        }

        old->_left->set_parent(n);
        if (old->_right) {
            old->_right->set_parent(n);
        }
        goto color;
    }

    parent = n->parent();
    color = n->color();

    if (child) {
        child->set_parent(parent);
    }
    if (parent) {
        if (parent->_left == n) {
            parent->_left = child;
        }
        else {
            parent->_right = child;
        }
    }
    else {
        _root = child;
    }

color:
    if (color == node::black) {
        erase_color(child, parent);
    }
}

void rbtree::replace(node *victim, node *new_node)
{
    node *parent = victim->parent();

    /* Set the surrounding nodes to point to the replacement */
    if (parent) {
        if (victim == parent->_left) {
            parent->_left = new_node;
        } 
        else {
            parent->_right = new_node;
        }
    }
    else {
        _root = new_node;
    }
    if (victim->_left) {
        victim->_left->set_parent(new_node);
    }
    if (victim->_right) {
        victim->_right->set_parent(new_node);
    }

    /* Copy the pointers/colour from the victim to the replacement */
    *new_node = *victim;
}

}
