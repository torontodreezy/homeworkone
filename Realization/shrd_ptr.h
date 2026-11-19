#ifndef SHRD_PTR_H
#define SHRD_PTR_H

#include "ControlBlock.h"
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<class T> class WeakPtr;
template<class T> class ShrdPtr;
template<class T, class... Args> ShrdPtr<T> makeShrd(Args&&... args);
template<class T> ShrdPtr<T[]> makeShrdArray(std::size_t count);

// Владение одиночным объектом.
template<class T>
class ShrdPtr {
    template<class> friend class ShrdPtr;
    template<class> friend class WeakPtr;
    template<class U, class... Args> friend ShrdPtr<U> makeShrd(Args&&... args);

    ControlBlock* control = nullptr;
    T* pointer = nullptr;

    // Сырой указатель принимают фабрика и reset; обычное создание — через UnqPtr*.
    explicit ShrdPtr(T* p)
        : control(ControlBlock::fromRaw<T>(p)), pointer(p) {}

    void release() noexcept {
        ControlBlock* old = control;
        control = nullptr;
        pointer = nullptr;
        if (old != nullptr) old->decStrong();
    }

public:
    ShrdPtr() noexcept = default;
    ShrdPtr(std::nullptr_t) noexcept {}

    explicit ShrdPtr(UnqPtr<T>* owner)
        : control(ControlBlock::create(owner)),
          pointer(owner == nullptr ? nullptr : owner->get()) {}

    ShrdPtr(const ShrdPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->incStrong();
    }

    // Разрешаем только безопасное преобразование адресов.
    template<class U, class = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr(const ShrdPtr<U>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->incStrong();
    }

    ShrdPtr(ShrdPtr&& other) noexcept
        : control(other.control), pointer(other.pointer) {
        other.control = nullptr;
        other.pointer = nullptr;
    }

    ~ShrdPtr() { release(); }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        ShrdPtr copy(other);
        swap(copy);
        return *this;
    }

    template<class U, class = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr& operator=(const ShrdPtr<U>& other) noexcept {
        ShrdPtr copy(other);
        swap(copy);
        return *this;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            ShrdPtr moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    ShrdPtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    T* get() const noexcept { return pointer; }
    explicit operator bool() const noexcept { return pointer != nullptr; }
    std::size_t use_count() const noexcept {
        return control == nullptr ? 0 : control->useCount();
    }

    T& operator*() const {
        if (pointer == nullptr) throw std::runtime_error("dereference null ShrdPtr");
        return *pointer;
    }
    T* operator->() const noexcept { return pointer; }

    void reset() noexcept { release(); }
    void reset(std::nullptr_t) noexcept { reset(); }

    template<class U, class = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    void reset(U* p) {
        // Передавать можно только новый адрес, которым ещё никто не владеет.
        ShrdPtr replacement;
        replacement.control = ControlBlock::fromRaw<U>(p);
        replacement.pointer = p;
        swap(replacement);
    }

    void swap(ShrdPtr& other) noexcept {
        std::swap(control, other.control);
        std::swap(pointer, other.pointer);
    }
};

// Владение массивом.
template<class T>
class ShrdPtr<T[]> {
    template<class> friend class ShrdPtr;
    template<class> friend class WeakPtr;
    template<class U> friend ShrdPtr<U[]> makeShrdArray(std::size_t count);

    ControlBlock* control = nullptr;
    T* pointer = nullptr;

    // Сырой указатель принимают фабрика и reset; обычное создание — через UnqPtr*.
    explicit ShrdPtr(T* p)
        : control(ControlBlock::fromRaw<T[]>(p)), pointer(p) {}

    void release() noexcept {
        ControlBlock* old = control;
        control = nullptr;
        pointer = nullptr;
        if (old != nullptr) old->decStrong();
    }

public:
    ShrdPtr() noexcept = default;
    ShrdPtr(std::nullptr_t) noexcept {}

    explicit ShrdPtr(UnqPtr<T[]>* owner)
        : control(ControlBlock::create(owner)),
          pointer(owner == nullptr ? nullptr : owner->get()) {}

    ShrdPtr(const ShrdPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->incStrong();
    }

    // Разрешаем только безопасное преобразование адресов.
    template<class U, class = std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>>>
    ShrdPtr(const ShrdPtr<U[]>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->incStrong();
    }

    ShrdPtr(ShrdPtr&& other) noexcept
        : control(other.control), pointer(other.pointer) {
        other.control = nullptr;
        other.pointer = nullptr;
    }

    ~ShrdPtr() { release(); }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        ShrdPtr copy(other);
        swap(copy);
        return *this;
    }

    template<class U, class = std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>>>
    ShrdPtr& operator=(const ShrdPtr<U[]>& other) noexcept {
        ShrdPtr copy(other);
        swap(copy);
        return *this;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            ShrdPtr moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    ShrdPtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    T* get() const noexcept { return pointer; }
    explicit operator bool() const noexcept { return pointer != nullptr; }
    std::size_t use_count() const noexcept {
        return control == nullptr ? 0 : control->useCount();
    }

    T& operator[](std::size_t index) const { return pointer[index]; }

    void reset() noexcept { release(); }

    void swap(ShrdPtr& other) noexcept {
        std::swap(control, other.control);
        std::swap(pointer, other.pointer);
    }
};

// Создание объекта с передачей аргументов его конструктору.
template<class T, class... Args>
ShrdPtr<T> makeShrd(Args&&... args) {
    static_assert(!std::is_array_v<T>, "Use makeShrdArray for arrays");
    return ShrdPtr<T>(new T(std::forward<Args>(args)...));
}

// Скобки () обнуляют элементы встроенных типов.
template<class T>
ShrdPtr<T[]> makeShrdArray(std::size_t count) {
    return ShrdPtr<T[]>(new T[count]());
}

#endif
