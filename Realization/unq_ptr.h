#ifndef UNQ_PTR_H
#define UNQ_PTR_H

#include <cstddef>
#include <type_traits>
#include <utility>

namespace pointer_detail {
    template <class From, class To>
    concept Compatible = (std::is_array_v<From> == std::is_array_v<To>) &&
        std::is_convertible_v<From*, To*>;

    template <class From, class To>
    concept UniqueCompatible = Compatible<From, To> &&
    (std::is_array_v<To> ||
        std::is_same_v<std::remove_cv_t<From>, std::remove_cv_t<To>> ||
        std::has_virtual_destructor_v<To>);

    // Тип владения для сырого указателя на объект или элемент массива.
    template <class Owner, class U>
    using RawOwner = std::conditional_t<std::is_array_v<Owner>, U[], U>;
}

template <class T>
class UnqPtr {
    static_assert(!std::is_bounded_array_v<T>, "Use UnqPtr<T[]> for arrays");
    using Element = std::remove_extent_t<T>;

    Element* pointer = nullptr;

    void replace(Element* newPointer) noexcept {
        if (pointer == newPointer) return;

        Element* old = std::exchange(pointer, newPointer);
        if constexpr (std::is_array_v<T>) {
            delete[] old;
        }
        else {
            delete old;
        }
    }

public:
    UnqPtr() noexcept = default;
    explicit UnqPtr(std::nullptr_t) noexcept {}

    template <class U>
        requires pointer_detail::UniqueCompatible<pointer_detail::RawOwner<T, U>, T>
    explicit UnqPtr(U* ptr) noexcept : pointer(ptr) {}

    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr(UnqPtr&& other) noexcept : pointer(other.release()) {}

    // При переходе к базовому типу удалять объект будет деструктор базы.
    template <class U>
        requires pointer_detail::UniqueCompatible<U, T>
    UnqPtr(UnqPtr<U>&& other) noexcept : pointer(other.release()) {}

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) replace(other.release());
        return *this;
    }

    template <class U>
        requires pointer_detail::UniqueCompatible<U, T>
    UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
        replace(other.release());
        return *this;
    }

    ~UnqPtr() {
        reset();
    }

    Element* get() const noexcept {
        return pointer;
    }

    Element* release() noexcept {
        return std::exchange(pointer, nullptr);
    }

    void reset(std::nullptr_t = nullptr) noexcept {
        replace(nullptr);
    }

    template <class U>
        requires pointer_detail::UniqueCompatible<pointer_detail::RawOwner<T, U>, T>
    void reset(U* ptr) noexcept {
        replace(ptr);
    }

    Element& operator[](std::size_t index) const requires std::is_array_v<T> {
        return pointer[index];
    }
};

#endif
