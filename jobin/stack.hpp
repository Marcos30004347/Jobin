#define JOBIN_STACK_H
#ifdef JOBIN_STACK_H

#include "spin_lock.hpp"

template<typename T>
class stack {
    private:
    spin_lock q_mutex;

    struct node;
    node* _top;
    unsigned int _size;

    public:
    stack();
    ~stack();

    void push_top(const T& value);

    bool pop_top();

    T& top();

    unsigned int size();
};


template<typename T>
struct stack<T>::node {
    T data;
    node* parent;
    node(const T& value): parent { nullptr }, data { value } {}
};

template<typename T>
stack<T>::stack(): _size{0}, q_mutex{}, _top {nullptr} {}

template<typename T>
stack<T>::~stack() {}

template<typename T>
void stack<T>::push_top(const T& value) {    
    node* new_node = new node(value);

    q_mutex.lock();

    new_node->parent = _top;
    _top = new_node;
    _size++;
    q_mutex.unlock();

}

template<typename T>
bool stack<T>::pop_top() {
    node* tmp = nullptr;

    q_mutex.lock();
    tmp = _top;

    if(!tmp) {
        q_mutex.unlock();
        return false;
    }

    _top = tmp->parent;
    _size--;
    q_mutex.unlock();
    
    delete tmp;
    return true;
}


template<typename T>
T& stack<T>::top() {
    return _top->data;
}

template<typename T>
unsigned int stack<T>::size() {
    return _size;
}


#endif