#ifndef UNQ_PTR_H
#define UNQ_PTR_H

#include <cstddef>

// Владение одним объектом.
template <class T>
class UnqPtr {
private:
    T* pointer;

public:
    UnqPtr() : pointer(nullptr) {}

    explicit UnqPtr(T* ptr) : pointer(ptr) {}

    // Копирование запрещено.
    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    // Перемещение между одинаковыми типами.
    UnqPtr(UnqPtr&& other) noexcept
        : pointer(other.release()) {}

    // Перемещение от наследника к базовому типу.
    template <class U>
    UnqPtr(UnqPtr<U>&& other) noexcept
        : pointer(nullptr) {
        U* ptr = other.get();
        pointer = ptr;

        other.release();
    }

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }

        return *this;
    }

    template <class U>
    UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
        U* source = other.get();
        T* ptr = source;

        other.release();
        reset(ptr);

        return *this;
    }

    ~UnqPtr() {
        delete pointer;
    }

    T* get() const {
        return pointer;
    }

    // Отдаём адрес без удаления объекта.
    T* release() {
        T* ptr = pointer;
        pointer = nullptr;

        return ptr;
    }

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
};





// Владение массивом.
template <class T>
class UnqPtr<T[]> {
private:
    T* pointer;

public:
    UnqPtr() : pointer(nullptr) {}

    explicit UnqPtr(T* ptr) : pointer(ptr) {}

    // Не принимаем массивы с другим типом элементов.
    template <class U>
    UnqPtr(U*) = delete;

    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr(UnqPtr&& other) noexcept
        : pointer(other.release()) {}

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }

        return *this;
    }

    ~UnqPtr() {
        delete[] pointer;
    }

    T* get() const {
        return pointer;
    }

    T& operator[](std::size_t index) const {
        return pointer[index];
    }

    T* release() {
        T* ptr = pointer;
        pointer = nullptr;

        return ptr;
    }

    void reset() {
        delete[] pointer;
        pointer = nullptr;
    }

    void reset(T* ptr) {
        if (pointer != ptr) {
            delete[] pointer;
            pointer = ptr;
        }
    }

    template <class U>
    void reset(U*) = delete;
};

#endif
