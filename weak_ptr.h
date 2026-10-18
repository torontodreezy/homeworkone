#ifndef WEAK_PTR_H
#define WEAK_PTR_H

#include "shrd_ptr.h"

// Слабый указатель на одиночный объект.
template <class T>
class WeakPtr {
    template <class U>
    friend class WeakPtr;

private:
    ShrdControl* control;
    ShrdAccess<T>* access;

    void release() {
        ShrdControl* oldControl = control;
        ShrdAccess<T>* oldAccess = access;

        control = nullptr;
        access = nullptr;

        delete oldAccess;

        if (oldControl != nullptr) {
            --oldControl->weakCount;

            if (oldControl->weakCount == 0) {
                delete oldControl;
            }
        }
    }

    void swap(WeakPtr& other) {
        ShrdControl* oldControl = control;
        control = other.control;
        other.control = oldControl;

        ShrdAccess<T>* oldAccess = access;
        access = other.access;
        other.access = oldAccess;
    }

public:
    WeakPtr() : control(nullptr), access(nullptr) {}

    WeakPtr(const ShrdPtr<T>& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = other.access->copy();
            ++control->weakCount;
        }
    }

    template <class U>
    WeakPtr(const ShrdPtr<U>& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = new ShrdConvertedAccess<T, U>(
                other.access
            );

            ++control->weakCount;
        }
    }

    WeakPtr(const WeakPtr& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = other.access->copy();
            ++control->weakCount;
        }
    }

    template <class U>
    WeakPtr(const WeakPtr<U>& other)
        : control(other.control), access(nullptr) {
        if (control != nullptr) {
            access = new ShrdConvertedAccess<T, U>(
                other.access
            );

            ++control->weakCount;
        }
    }

    WeakPtr(WeakPtr&& other) noexcept
        : control(other.control), access(other.access) {
        other.control = nullptr;
        other.access = nullptr;
    }

    ~WeakPtr() {
        release();
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this != &other) {
            WeakPtr copy(other);
            swap(copy);
        }

        return *this;
    }

    template <class U>
    WeakPtr& operator=(const WeakPtr<U>& other) {
        WeakPtr copy(other);
        swap(copy);

        return *this;
    }

    template <class U>
    WeakPtr& operator=(const ShrdPtr<U>& other) {
        WeakPtr copy(other);
        swap(copy);

        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this != &other) {
            release();

            control = other.control;
            access = other.access;

            other.control = nullptr;
            other.access = nullptr;
        }

        return *this;
    }

    void reset() {
        release();
    }

    std::size_t use_count() const {
        return control == nullptr ? 0 : control->count;
    }

    bool expired() const {
        return use_count() == 0;
    }

    ShrdPtr<T> lock() const {
        ShrdPtr<T> result;

        if (!expired()) {
            result.access = access->copy();
            result.control = control;

            ++control->count;
        }

        return result;
    }
};

// Слабый указатель на массив.
template <class T>
class WeakPtr<T[]> {
private:
    ShrdOwner<T[]>* control;

    void release() {
        ShrdOwner<T[]>* oldControl = control;
        control = nullptr;

        if (oldControl != nullptr) {
            --oldControl->weakCount;

            if (oldControl->weakCount == 0) {
                delete oldControl;
            }
        }
    }

    void swap(WeakPtr& other) {
        ShrdOwner<T[]>* oldControl = control;
        control = other.control;
        other.control = oldControl;
    }

public:
    WeakPtr() : control(nullptr) {}

    WeakPtr(const ShrdPtr<T[]>& other)
        : control(other.control) {
        if (control != nullptr) {
            ++control->weakCount;
        }
    }

    WeakPtr(const WeakPtr& other)
        : control(other.control) {
        if (control != nullptr) {
            ++control->weakCount;
        }
    }

    WeakPtr(WeakPtr&& other) noexcept
        : control(other.control) {
        other.control = nullptr;
    }

    ~WeakPtr() {
        release();
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this != &other) {
            WeakPtr copy(other);
            swap(copy);
        }

        return *this;
    }

    WeakPtr& operator=(const ShrdPtr<T[]>& other) {
        WeakPtr copy(other);
        swap(copy);

        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this != &other) {
            release();

            control = other.control;
            other.control = nullptr;
        }

        return *this;
    }

    void reset() {
        release();
    }

    std::size_t use_count() const {
        return control == nullptr ? 0 : control->count;
    }

    bool expired() const {
        return use_count() == 0;
    }

    ShrdPtr<T[]> lock() const {
        ShrdPtr<T[]> result;

        if (!expired()) {
            result.control = control;
            ++control->count;
        }

        return result;
    }
};

#endif
