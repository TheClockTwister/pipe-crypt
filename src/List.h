#ifndef _LIST_H
#define _LIST_H

#include "Node.h"

#include <iostream>
#include <string>

template<class T>
class List {
    private:
        int length = 0;
        Node<T>* head = nullptr;
        Node<T>* tail = nullptr;

    public:

    int size(){
        return length;
    }

    void put(T data){
        if (length == 0){ // if empty
            head = new Node<T>(data);
            tail = head;
        } else {
            head->next = new Node<T>(data);
            head->next->prev = head;
            head = head->next;
        }
        length++;
    }

    T pop(){
        if (length == 0){ throw std::range_error("List is empty"); }

        Node<T> item = *tail;
        tail = item.next;
        if (tail) tail->prev = nullptr;
        length--;
        return std::move(item.data);
    }

    Node<T>* front(){ return tail; }
    Node<T>* back(){ return head; }

};

#endif
