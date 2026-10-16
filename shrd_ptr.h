#ifndef SHRD_PTR_H
#define SHRD_PTR_H

#include "shrd_control.h"

// Дескриптор одиночного объекта.
template <class T>
class ShrdPtr {
    template <class U>
    friend class ShrdPtr;

private:
    ShrdControl* control;
    ShrdAccess<T>* access;

    void release() {
        if (control != nullptr) {
            delete access;

            --control->count;

            if (control->count == 0) {
                delete control;
            }
        }

        control = nullptr;
        access = nullptr;
    }

    void swap(ShrdPtr& other) {
        ShrdControl* oldControl = control;
        control = other.control;
        other.control = oldControl;

        ShrdAccess<T>* oldAccess = access;
        access = other.access;
        other.access = oldAccess;
    }

public:
    ShrdPtr() : control(nullptr), access(nullptr) {}

    explicit ShrdPtr(UnqPtr<T>* ptr)
        : control(nullptr), access(nullptr) {
        if (ptr != nullptr) {
            ShrdOwner<T>* owner = nullptr;

            try {
                owner = new ShrdOwner<T>(ptr);
                access = new ShrdDirectAccess<T>(owner);
            }
            catch (...) {
                if (owner != nullptr) {
                    delete owner;
                }
                else {
                    delete ptr;
                }

                throw;
            }

            control = owner;
        }
    }

    ShrdPtr(const ShrdPtr& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = other.access->copy();
            ++control->count;
        }
    }

    template <class U>
    ShrdPtr(const ShrdPtr<U>& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = new ShrdConvertedAccess<T, U>(
                other.access
            );

            ++control->count;
        }
    }

    ~ShrdPtr() {
        release();
    }

    ShrdPtr& operator=(const ShrdPtr& other) {
        if (this != &other) {
            ShrdPtr copy(other);
            swap(copy);
        }

        return *this;
    }

    template <class U>
    ShrdPtr& operator=(const ShrdPtr<U>& other) {
        ShrdPtr copy(other);
        swap(copy);

        return *this;
    }

    T* get() const {
        if (access == nullptr) {
            return nullptr;
        }

        return access->get();
    }
};

// Дескриптор массива.
template <class T>
class ShrdPtr<T[]> {
private:
    ShrdOwner<T[]>* control;

    void release() {
        if (control != nullptr) {
            --control->count;

            if (control->count == 0) {
                delete control;
            }
        }

        control = nullptr;
    }

public:
    ShrdPtr() : control(nullptr) {}

    explicit ShrdPtr(UnqPtr<T[]>* ptr)
        : control(nullptr) {
        if (ptr != nullptr) {
            try {
                control = new ShrdOwner<T[]>(ptr);
            }
            catch (...) {
                delete ptr;
                throw;
            }
        }
    }

    ShrdPtr(const ShrdPtr& other)
        : control(other.control) {
        if (control != nullptr) {
            ++control->count;
        }
    }

    ~ShrdPtr() {
        release();
    }

    ShrdPtr& operator=(const ShrdPtr& other) {
        if (this != &other) {
            ShrdOwner<T[]>* next = other.control;

            if (next != nullptr) {
                ++next->count;
            }

            release();
            control = next;
        }

        return *this;
    }

    T* get() const {
        if (control == nullptr) {
            return nullptr;
        }

        return control->getMaster()->get();
    }

    T& operator[](std::size_t index) const {
        return get()[index];
    }
};

#endif
