#ifndef LIST_HPP
#define LIST_HPP

#include "MemoryAllocator.hpp"

template <typename T>
class List {
private:
    // Unutrasnja struktura koja predstavlja jedan cvor u listi
    struct Node {
        T* data;     // Pokazivac na podatak (npr. TCB*)
        Node* next;  // Pokazivac na sledeci cvor
        Node* prev;  // Pokazivac na prethodni cvor

        Node(T* data) : data(data), next(nullptr), prev(nullptr) {}


        void* operator new(size_t size) {
            size_t blocks = size / MEM_BLOCK_SIZE;
            if (size % MEM_BLOCK_SIZE != 0) blocks++;
            return MemoryAllocator::mem_alloc(blocks);
        }
        void operator delete(void* ptr) {
            MemoryAllocator::mem_free(ptr);
        }
    };

    Node* head; // Pokazivac na prvi element
    Node* tail; // Pokazivac na poslednji element

public:
    List() : head(nullptr), tail(nullptr) {}

    ~List() {
        while (head != nullptr) {
            removeFirst();
        }
    }

    // Dodaj element na kraj liste
    void putBack(T* item) {
        Node* newNode = new Node(item);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    // Dodaj element na pocetak liste
    void putFront(T* item) {
        Node* newNode = new Node(item);
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
    }


    T* peekFirst() const {
        if (head == nullptr) return nullptr;
        return head->data;
    }


    T* removeFirst() {
        if (head == nullptr) return nullptr;

        Node* oldHead = head;
        T* data = oldHead->data;

        head = head->next;
        if (head == nullptr) {
            tail = nullptr;
        } else {
            head->prev = nullptr;
        }

        delete oldHead;
        return data;
    }

    void remove(T* item) {
        Node* curr = head;
        while (curr != nullptr) {
            if (curr->data == item) {
                if (curr->prev != nullptr) {
                    curr->prev->next = curr->next;
                } else {
                    head = curr->next; // Bio je head
                }

                if (curr->next != nullptr) {
                    curr->next->prev = curr->prev;
                } else {
                    tail = curr->prev; //Bio je tail
                }

                delete curr;
                return;
            }
            curr = curr->next;
        }
    }
};

#endif