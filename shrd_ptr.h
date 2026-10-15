#ifndef SHRD_PTR_H
#define SHRD_PTR_H

#include "shrd_control.h"

template <class T>
class ShrdPtr {
    // Нужно для работы между ShrdPtr разных типов.
    template <class U>
    friend class ShrdPtr;

private:
    ShrdControl* control;
    ShrdAccess<T>* access;

    // Отказываемся от текущего владения.
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

    // Обмениваем содержимое двух дескрипторов.
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

    // Принимаем владение UnqPtr, созданным через new.
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

    // Копирование дескриптора того же типа.
    ShrdPtr(const ShrdPtr& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = other.access->copy();
            ++control->count;
        }
    }

    // Копирование с переходом к базовому типу.
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

    // Присваивание между одинаковыми типами.
    ShrdPtr& operator=(const ShrdPtr& other) {
        if (this != &other) {
            ShrdPtr copy(other);
            swap(copy);
        }

        return *this;
    }

    // Присваивание с переходом к базовому типу.
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

#endif
