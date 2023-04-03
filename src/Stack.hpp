#pragma once

template <typename T>
class Stack
{
private:
public:
    T *list;
    int height;
    int size;

    Stack();
    ~Stack();

    void alloc(int size);
    T *getList();

    int getHeight();

    void push(T obj);
    T pop();
    T peek();

    void remove(T obj);
    void clear();
};





template <typename T>
Stack<T>::Stack() : size(0), height(0), list(nullptr) {}
template <typename T>
Stack<T>::~Stack()
{
    if (list != nullptr)
        delete[] list;
    list = nullptr;
}

template <typename T>
void Stack<T>::alloc(int size)
{
    if (this->size > size)
        return;

    this->size = size;
    if (list != nullptr)
        delete[] list;
    list = new T[size];
}

template <typename T>
T *Stack<T>::getList() { return list; }

template <typename T>
inline int Stack<T>::getHeight() { return height; }

template <typename T>
inline void Stack<T>::push(T obj)
{
    list[height] = obj;
    height += height < (size - 1);
}
template <typename T>
inline T Stack<T>::pop()
{
    height -= height > 0;
    return list[height];
}
template <typename T>
inline T Stack<T>::peek()
{
    return (height != 0) * list[height - 1];
    // if (height == 0)
    // return nullptr;
    // return list[height - 1];
}

template <typename T>
inline void Stack<T>::remove(T obj)
{
    int i;
    for (i = 0; i < height; i++)
    {
        if (list[i] == obj)
            break;
    }

    list[i] = pop();
}

template <typename T>
inline void Stack<T>::clear()
{
    height = 0;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, Stack<T> *&stack)
{
    os << "{ ";
    for (int i = 0; i < stack->height - 1; i++)
    {
        os << stack->list[i] << ", ";
    }
    if(stack.height > 0)
        os << stack->list[stack->height - 1];
    return os << " }";
}

template <typename T>
std::ostream &operator<<(std::ostream &os, Stack<T> &stack)
{
    os << "{ ";
    for (int i = 0; i < stack.height - 1; i++)
    {
        os << stack.list[i] << ", ";
    }
    if(stack.height > 0)
        os << stack.list[stack.height - 1];
    return os << " }";
}


