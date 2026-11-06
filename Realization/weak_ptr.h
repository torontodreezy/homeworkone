#ifndef WEAK_PTR_H
#define WEAK_PTR_H

#include "shrd_ptr.h"

template<class From, class To>
struct WeakCompatible : std::bool_constant<
    !std::is_array_v<From> && !std::is_array_v<To> &&
    std::is_convertible_v<From*, To*>> {};

template<class From, class To>
struct WeakCompatible<From[], To[]>
    : std::bool_constant<std::is_convertible_v<From(*)[], To(*)[]>> {};

template<class T>
class WeakPtr {
    template<class> friend class WeakPtr;
    using Element = std::remove_extent_t<T>;

    ControlBlock* control = nullptr;
    Element* pointer = nullptr;

    void release() noexcept {
        ControlBlock* old = control;
        control = nullptr;
        pointer = nullptr;
        if (old != nullptr) {
            old->decWeak();
        }
    }

public:
    WeakPtr() noexcept = default;

    template<class U, class = std::enable_if_t<WeakCompatible<U, T>::value>>
    WeakPtr(const ShrdPtr<U>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->incWeak();
    }

    WeakPtr(const WeakPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->incWeak();
    }

    template<class U, class = std::enable_if_t<WeakCompatible<U, T>::value>>
    WeakPtr(const WeakPtr<U>& other) noexcept : control(other.control) {
        // После удаления объекта нельзя вычислять адрес его виртуальной базы.
        if (control != nullptr) {
            if (control->useCount() != 0) pointer = other.pointer;
            control->incWeak();
        }
    }

    WeakPtr(WeakPtr&& other) noexcept
        : control(other.control), pointer(other.pointer) {
        other.control = nullptr;
        other.pointer = nullptr;
    }
    ~WeakPtr() { release(); }

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }
    template<class U, class = std::enable_if_t<WeakCompatible<U, T>::value>>
    WeakPtr& operator=(const WeakPtr<U>& other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }
    template<class U, class = std::enable_if_t<WeakCompatible<U, T>::value>>
    WeakPtr& operator=(const ShrdPtr<U>& other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this != &other) {
            WeakPtr(std::move(other)).swap(*this);
        }
        return *this;
    }

    void swap(WeakPtr& other) noexcept {
        std::swap(control, other.control);
        std::swap(pointer, other.pointer);
    }
    void reset() noexcept { release(); }
    std::size_t use_count() const noexcept {
        return control == nullptr ? 0 : control->useCount();
    }
    bool expired() const noexcept { return use_count() == 0; }

    ShrdPtr<T> lock() const noexcept {
        ShrdPtr<T> result;
        if (expired()) return result;

        control->incStrong();
        result.control = control;
        result.pointer = pointer;
        return result;
    }
};

#endif
