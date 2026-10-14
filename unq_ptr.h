//
// Created by bladerunner on 10/14/26.
//unique ptr realisation

#ifndef HOMEWORKONE_3_UNQ_PTR_H
#define HOMEWORKONE_3_UNQ_PTR_H

template <class T>
class UnqPtr {
private:
    T* pointer;

public:
    explicit UnqPtr(T* ptr) : pointer(ptr) {}
    T* get() const {
        return pointer;
    }
    T& operator*() {
        return *pointer;
    }
    UnqPtr(): pointer(nullptr){}

    void reset() {
        delete pointer;
        pointer = nullptr;
    }

    void reset(T* ptr) {
        if (pointer != ptr) {
            delete pointer;
            pointer = ptr;
        }
    }

    //подтипизация
    template<class U>
    


    ~UnqPtr() {
        delete pointer;
    }

    UnqPtr(const UnqPtr& other): pointer(nullptr) {
        if (other.pointer != nullptr) {
            pointer = new T(*other.pointer);
        }

    }

    UnqPtr& operator=(const UnqPtr& other) {
        if (this != &other) {
            T* ptr = nullptr;
            if (other.pointer != nullptr) {
                ptr = new T(*other.pointer);
            }
            delete pointer;
            pointer = ptr;
        }
        return *this;
    }


};




#endif //HOMEWORKONE_3_UNQ_PTR_H
