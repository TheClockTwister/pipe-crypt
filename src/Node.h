#ifndef _NODE_H
#define _NODE_H

template<class T>
class Node {
public:
    T data;
    Node* next, * prev;

    Node(T data_): data(data_) {};

    Node(T data_, Node* next_, Node* prev_): data(data_), prev(prev_), next(next_) {};

    ~Node(){};
};

#endif