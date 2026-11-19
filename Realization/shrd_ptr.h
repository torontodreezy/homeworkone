#ifndef SHRD_PTR_H
#define SHRD_PTR_H

#include "ControlBlock.h"
#include <cstddef>
#include <stdexcept>

template<class T> class WeakPtr;

template<class T>
class ShrdPtr {
    static_assert(!std::is_bounded_array_v<T>, "Use ShrdPtr<T[]> for arrays");

    template<class> friend class ShrdPtr;
    template<class> friend class WeakPtr;

    using Element = std::remove_extent_t<T>;

    ControlBlock* control = nullptr;
    Element* pointer = nullptr;

    ShrdPtr(ControlBlock* block, Element* address) noexcept
        : control(block), pointer(address) {
        if (control != nullptr) control->incStrong();
    }

public:
    ShrdPtr() noexcept = default;
    ShrdPtr(std::nullptr_t) noexcept {}

    template<class U>
        requires pointer_detail::Compatible<U, T>
    explicit ShrdPtr(UnqPtr<U>&& owner) : pointer(owner.get()) {
        // Блок запоминает тип U, по которому впоследствии удалит объект.
        control = ControlBlock::create(std::move(owner));
    }

    ShrdPtr(const ShrdPtr& other) noexcept
        : ShrdPtr(other.control, other.pointer) {}

    template<class U>
        requires pointer_detail::Compatible<U, T>
    ShrdPtr(const ShrdPtr<U>& other) noexcept
        : ShrdPtr(other.control, other.pointer) {}

    ShrdPtr(ShrdPtr&& other) noexcept
        : control(std::exchange(other.control, nullptr)),
          pointer(std::exchange(other.pointer, nullptr)) {}

    template<class U>
        requires pointer_detail::Compatible<U, T>
    ShrdPtr(ShrdPtr<U>&& other) noexcept
        : control(std::exchange(other.control, nullptr)),
          pointer(std::exchange(other.pointer, nullptr)) {}

    ~ShrdPtr() { reset(); }

    // Параметр принимает нового владельца до уничтожения старого.
    ShrdPtr& operator=(ShrdPtr other) noexcept {
        swap(other);
        return *this;
    }

    Element* get() const noexcept { return pointer; }
    explicit operator bool() const noexcept { return pointer != nullptr; }

    std::size_t use_count() const noexcept {
        return control == nullptr ? 0 : control->useCount();
    }

    Element& operator*() const requires (!std::is_array_v<T>) {
        if (pointer == nullptr) throw std::runtime_error("dereference null ShrdPtr");
        return *pointer;
    }

    Element* operator->() const noexcept requires (!std::is_array_v<T>) {
        return pointer;
    }

    Element& operator[](std::size_t index) const requires std::is_array_v<T> {
        return pointer[index];
    }

    void reset(std::nullptr_t = nullptr) noexcept {
        ControlBlock* old = std::exchange(control, nullptr);
        pointer = nullptr;
        if (old != nullptr) old->decStrong();
    }

    template<class U>
        requires pointer_detail::Compatible<pointer_detail::RawOwner<T, U>, T>
    void reset(U* p) {
        if (p == nullptr) {
            reset();
            return;
        }

        // По этому адресу ещё не должно быть владельца.
        using Owner = pointer_detail::RawOwner<T, U>;
        ShrdPtr(UnqPtr<Owner>{p}).swap(*this);
    }

    void swap(ShrdPtr& other) noexcept {
        std::swap(control, other.control);
        std::swap(pointer, other.pointer);
    }
};

template<class T, class... Args>
ShrdPtr<T> makeShrd(Args&&... args) {
    static_assert(!std::is_array_v<T>, "Use makeShrdArray for arrays");
    return ShrdPtr<T>(UnqPtr<T>{new T(std::forward<Args>(args)...)});
}

template<class T>
ShrdPtr<T[]> makeShrdArray(std::size_t count) {
    return ShrdPtr<T[]>(UnqPtr<T[]>{new T[count]()});
}

#endif
