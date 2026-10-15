
#ifndef HOMEWORKONE_3_SHRD_PTR_H
#define HOMEWORKONE_3_SHRD_PTR_H

#include "unq_ptr.h"
#include <cstddef>

template <class T>
class ShrdPtr {
    private:
    UnqPtr<T>* pointer;
    std::size_t* count;

    public:
    ShrdPtr(): pointer(nullptr), count(nullptr){}



};


#endif //HOMEWORKONE_3_SHRD_PTR_H
