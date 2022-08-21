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

    void put(T&& data){
        if (length == 0){ // if empty
            head = new Node<T>(std::move(data));
            tail = head;
        } else {
            head->next = new Node<T>(std::move(data));
            head = head->next;
        }
        length++;
    }

    T pop(){
        if (length == 0){ throw std::range_error("List is empty"); }

        Node<T>* item = tail;

        if (length == 1){
            tail = nullptr;
            head = nullptr;
        } else {
            tail = item->next;
        }

        length--;
        item->next = nullptr;
        T data = std::move(item->data);
        delete item;
        return std::move(data);
    }

    Node<T>* front(){ return tail; }
    Node<T>* back(){ return head; }

};

#endif
