#ifndef SHRD_CONTROL_H
#define SHRD_CONTROL_H

#include <cstddef>
#include "unq_ptr.h"

// Общий счётчик дескрипторов.
class ShrdControl {
public:
    std::size_t count;

    ShrdControl() : count(1) {}

    virtual ~ShrdControl() {}
};

// Хранит ведущий UnqPtr конкретного типа.
template <class T>
class ShrdOwner : public ShrdControl {
private:
    UnqPtr<T>* pointer;

public:
    explicit ShrdOwner(UnqPtr<T>* ptr) : pointer(ptr) {}

    UnqPtr<T>* getMaster() const {
        return pointer;
    }

    ~ShrdOwner() override {
        delete pointer;
    }
};

// Интерфейс доступа к объекту.
template <class T>
class ShrdAccess {
public:
    virtual T* get() const = 0;

    virtual ShrdAccess<T>* copy() const = 0;

    virtual ~ShrdAccess() {}
};

// Доступ через исходный тип ведущего указателя.
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

// Доступ к тому же объекту через базовый тип.
// T — новый тип, U — исходный.
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
        // Компилятор проверяет преобразование U* в T*.
        return pointer->get();
    }

    ShrdAccess<T>* copy() const override {
        return new ShrdConvertedAccess<T, U>(pointer);
    }

    ~ShrdConvertedAccess() override {
        // Удаляем только вспомогательный объект доступа.
        delete pointer;
    }
};

#endif
