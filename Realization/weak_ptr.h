#ifndef WEAK_PTR_H
#define WEAK_PTR_H

#include "shrd_ptr.h"

template<class T>
class WeakPtr {
    template<class> friend class WeakPtr;
    using Element = typename std::remove_extent<T>::type;
    ControlBlock* control = nullptr;
    Element* pointer = nullptr;

    void release() noexcept {
        ControlBlock* old = control;
        control = nullptr;
        pointer = nullptr;
        if (old && --old->weakCount == 0) delete old;
    }
public:
    WeakPtr() noexcept = default;
    template<class U, typename std::enable_if<shrd_detail::compatible<U,T>::value, int>::type = 0>
    WeakPtr(const ShrdPtr<U>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control) ++control->weakCount;
    }
    WeakPtr(const WeakPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control) ++control->weakCount;
    }
    template<class U, typename std::enable_if<shrd_detail::compatible<U,T>::value, int>::type = 0>
    WeakPtr(const WeakPtr<U>& other) noexcept : control(other.control) {
        // Avoid pointer adjustment through an already destroyed virtual base.
        if (control) {
            if (control->count) pointer = other.pointer;
            ++control->weakCount;
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
    template<class U, typename std::enable_if<shrd_detail::compatible<U,T>::value, int>::type = 0>
    WeakPtr& operator=(const WeakPtr<U>& other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }
    template<class U, typename std::enable_if<shrd_detail::compatible<U,T>::value, int>::type = 0>
    WeakPtr& operator=(const ShrdPtr<U>& other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this != &other) { WeakPtr(std::move(other)).swap(*this); }
        return *this;
    }
    void swap(WeakPtr& other) noexcept {
        std::swap(control, other.control);
        std::swap(pointer, other.pointer);
    }
    void reset() noexcept { release(); }
    std::size_t use_count() const noexcept { return control ? control->count : 0; }
    bool expired() const noexcept { return use_count() == 0; }
    ShrdPtr<T> lock() const noexcept {
        ShrdPtr<T> result;
        if (!expired()) {
            result.control = control;
            result.pointer = pointer;
            ++control->count;
        }
        return result;
    }
};
#endif
