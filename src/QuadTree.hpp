#pragma once

#include "Stack.hpp"
#include "Vector.hpp"

typedef struct
{
    double left;
    double right;
    double bottom;
    double top;
} Rect;

// object is of type T, info element of type U always present
template <typename T, typename U>
class QuadTree
{
private:
public:
    Vector<2> center;
    Rect rect;
    double size;
    int depth;
    QuadTree<T, U> *parent;
    bool leaf;
    bool hasObj;
    int objCounter;
    T obj;
    U info;
    QuadTree<T, U> *children[4];
    Vector<2> objPosition;

    QuadTree(Vector<2> center,
             double size,
             int depth,
             QuadTree<T, U> *parent);

    ~QuadTree();


    void clear();

    void setObj(T obj, Vector<2> objPosition);
    void unsetObj();

    bool inBounds(Vector<2> position);
    int childIdx(Vector<2> position);
    Vector<2> childCenter(int i);

    QuadTree<T, U> *find(Vector<2> position);

    void split();
    void insert(T object, Vector<2> position);

    void merge();
    void remove(Vector<2> position);

    std::ostream &print(std::ostream &os, int depth, const char *prefix);
};

template <typename T, typename U>
void initPreOrder(Stack<QuadTree<T, U> *> *result, QuadTree<T, U> *root);
template <typename T, typename U>
QuadTree<T, U> *nextPreOrder(Stack<QuadTree<T, U> *> *state);

template <typename T, typename U>
void initPostOrder(Stack<QuadTree<T, U> *> *result, QuadTree<T, U> *root);

template <typename T, typename U>
QuadTree<T, U> *nextPostOrder(Stack<QuadTree<T, U> *> *state);

template <typename T, typename U>
std::ostream &operator<<(std::ostream &os, QuadTree<T, U> *&tree);

template <typename T, typename U>
std::ostream &operator<<(std::ostream &os, QuadTree<T, U> &tree);



constexpr int TREE_MAXDEPTH = 16;

static const Vector<2> childOffsets[] = {
    Vector<2>{-1.0, -1.0}, 
    Vector<2>{1.0, -1.0}, 
    Vector<2>{-1.0, 1.0}, 
    Vector<2>{1.0, 1.0} 
};


// object is of type T, info element of type U always present
template <typename T, typename U>
QuadTree<T,U>::QuadTree(Vector<2> center,
                   double size,
                   int depth,
                   QuadTree<T, U> *parent)
    : center(center),
      size(size),
      depth(depth),
      rect({center[0] - size, center[0] + size, center[1] - size, center[1] + size}),
      parent(parent),
      children{nullptr, nullptr, nullptr, nullptr},
      objCounter(0),
      objPosition(center),
      leaf(true),
      hasObj(false) {}

template <typename T, typename U>
QuadTree<T,U>::~QuadTree()
{
    for (int i = 0; i < 4; i++)
    {
        // if (children[i] != nullptr)
        // delete children[i];
    }
}

template <typename T, typename U>
inline bool QuadTree<T,U>::inBounds(Vector<2> position)
{
    bool x = rect.left <= position[0] && position[0] < rect.right;
    bool y = rect.bottom <= position[1] && position[1] < rect.top;
    return x && y;
}

template <typename T, typename U>
void QuadTree<T,U>::clear()
{
    if (hasObj)
        unsetObj();
    if (!leaf)
        for (int i = 0; i < 4; i++)
            children[i]->clear();
    objCounter = 0;
}

template <typename T, typename U>
inline void QuadTree<T,U>::setObj(T obj, Vector<2> objPosition)
{
    this->obj = obj;
    this->objPosition = objPosition;
    this->hasObj = true;
    this->objCounter = 1;
}
template <typename T, typename U>
inline void QuadTree<T,U>::unsetObj()
{
    this->hasObj = false;
    this->objCounter = 0;
}

template <typename T, typename U>
inline int QuadTree<T,U>::childIdx(Vector<2> position)
{
    bool east = position[0] > center[0];
    bool north = position[1] > center[1];
    int idx = 2 * north + east;
    return idx;
}

template <typename T, typename U>
inline Vector<2> QuadTree<T,U>::childCenter(int i)
{
    return (center + (childOffsets[i] * (size / 2)));
}

template <typename T, typename U>
QuadTree<T, U> *QuadTree<T,U>::find(Vector<2> position)
{
    if (leaf)
        return this;
    else
        return children[childIdx(position)]->find(position);
}

template <typename T, typename U>
void QuadTree<T,U>::split()
{
    if (children[0] == nullptr)
    {
        for (int i = 0; i < 4; i++)
        {
            children[i] = new QuadTree<T, U>(childCenter(i), size / 2, depth + 1, this);
        }
    }

    if (hasObj)
    {
        QuadTree<T, U> *child = children[childIdx(objPosition)];
        child->setObj(obj, objPosition);
    }

    hasObj = false;
    leaf = false;
}

template <typename T, typename U>
void QuadTree<T,U>::merge()
{
    for (int i = 0; i < 4; i++)
    {
        if (children[i]->hasObj)
            setObj(children[i]->obj, children[i]->objPosition);
        // delete children[i];
        children[i]->unsetObj();
    }
    leaf = true;
}

template <typename T, typename U>
inline void QuadTree<T,U>::insert(T object, Vector<2> position)
{
    objCounter++;
    if (leaf)
    {
        if (!hasObj)
        {
            setObj(object, position);
        }
        else if (depth < TREE_MAXDEPTH)
        {
            split();
            children[childIdx(position)]->insert(object, position);
        }
    }
    else
    {
        children[childIdx(position)]->insert(object, position);
    }
}

template <typename T, typename U>
inline void QuadTree<T,U>::remove(Vector<2> position)
{
    if (leaf)
    {
        unsetObj();
    }
    else
    {
        objCounter--;
        children[childIdx(position)]->remove(position);
        if (objCounter == 1)
        {
            merge();
        }
    }
}

template<typename T, typename U>
std::ostream &QuadTree<T,U>::print(std::ostream &os, int depth, const char *prefix)
{
    printf("\n");
    for (int i = 0; i < depth - 1; i++)
    {
        os << "│   ";
    }
    os << prefix;

    if (children[0] != nullptr)
    {
        os << this << " : " << center << "size = " << size << ", counter = " << objCounter;
        if (hasObj)
            os << ", object: " << obj << " at " << objPosition;
        for (int i = 0; i < 3; i++)
            children[i]->print(os, depth + 1, "├───");
        return children[3]->print(os, depth + 1, "└───");
    }
    else
    {
        return os << this << " : " << center << ", size = " << size << ",counter = " << objCounter;
        if (hasObj)
            return os << ", object: " << obj;
    }
};

template <typename T, typename U>
void initPreOrder(Stack<QuadTree<T, U> *> *result, QuadTree<T, U> *root)
{
    result->clear();
    result->push(root);
};
template <typename T, typename U>
QuadTree<T, U> *nextPreOrder(Stack<QuadTree<T, U> *> *state)
{
    QuadTree<T, U> *node = state->pop();
    if (!node->leaf)
    {
        for (int i = 0; i < 4; i++)
        {
            state->push(node->children[i]);
        }
    }
    return node;
};

template <typename T, typename U>
void initPostOrder(Stack<QuadTree<T, U> *> *result, QuadTree<T, U> *root)
{
    Stack<QuadTree<T, U> *> toVisit;
    toVisit.alloc(4 * root->objCounter);
    toVisit.push(root);
    result->clear();

    while (toVisit.getHeight() > 0 && result->getHeight() < result->size)
    {
        QuadTree<T, U> *current = toVisit.pop();
        result->push(current);

        if (!current->leaf)
        {
            for (int i = 0; i < 4; i++)
            {
                toVisit.push(current->children[i]);
            }
        }
    }
}

template <typename T, typename U>
QuadTree<T, U> *nextPostOrder(Stack<QuadTree<T, U> *> *state)
{
    return state->pop();
}

template <typename T, typename U>
std::ostream &operator<<(std::ostream &os, QuadTree<T, U> *&tree)
{
    return tree->print(os, 0, "");
}

template <typename T, typename U>
std::ostream &operator<<(std::ostream &os, QuadTree<T, U> &tree)
{
    return tree.print(os, 0, "");
}


