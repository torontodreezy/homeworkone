#ifndef SHRD_CONTROL_H
#define SHRD_CONTROL_H

#include <cstddef>
#include "unq_ptr.h"

class ShrdControl {
public:
    std::size_t count;
    std::size_t weakCount;

    ShrdControl() : count(1), weakCount(1) {}

    virtual void deleteObject() = 0;

    virtual ~ShrdControl() {}
};

template <class T>
class ShrdOwner : public ShrdControl {
private:
    UnqPtr<T>* pointer;

public:
    explicit ShrdOwner(UnqPtr<T>* ptr) : pointer(ptr) {}

    UnqPtr<T>* getMaster() const {
        return pointer;
    }

    void deleteObject() override {
        UnqPtr<T>* ptr = pointer;
        pointer = nullptr;

        delete ptr;
    }

    ~ShrdOwner() override {
        deleteObject();
    }
};

template <class T>
class ShrdAccess {
public:
    virtual T* get() const = 0;

    virtual ShrdAccess<T>* copy() const = 0;

    virtual ~ShrdAccess() {}
};

template <class T>
class ShrdDirectAccess : public ShrdAccess<T> {
private:
    ShrdOwner<T>* pointer;

public:
    explicit ShrdDirectAccess(ShrdOwner<T>* ptr)
        : pointer(ptr) {}

    T* get() const override {
        return pointer->getMaster()->get();
    }

    ShrdAccess<T>* copy() const override {
        return new ShrdDirectAccess<T>(pointer);
    }
};

template <class T, class U>
class ShrdConvertedAccess : public ShrdAccess<T> {
private:
    ShrdAccess<U>* pointer;

public:
    explicit ShrdConvertedAccess(const ShrdAccess<U>* ptr)
        : pointer(ptr->copy()) {}

    ShrdConvertedAccess(const ShrdConvertedAccess& other)
        : pointer(other.pointer->copy()) {}

    ShrdConvertedAccess& operator=(
        const ShrdConvertedAccess&
    ) = delete;

    T* get() const override {
        return pointer->get();
    }

    ShrdAccess<T>* copy() const override {
        return new ShrdConvertedAccess<T, U>(pointer);
    }

    ~ShrdConvertedAccess() override {
        delete pointer;
    }
};

#endif
