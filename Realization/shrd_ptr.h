#ifndef SHRD_PTR_H
#define SHRD_PTR_H

#include "unq_ptr.h"
#include <cstddef>
#include <type_traits>
#include <utility>

// weakCount includes one implicit reference while strong owners exist.
struct ControlBlock {
    std::size_t count = 1;
    std::size_t weakCount = 1;
    void* owner;
    void (*deleter)(void*) noexcept;

    ControlBlock(void* p, void (*d)(void*) noexcept) noexcept
        : owner(p), deleter(d) {}
};

namespace shrd_detail {
    template <class From, class To>
    struct compatible : std::integral_constant<bool,
                                               std::is_array<From>::value == std::is_array<To>::value &&
                                               std::conditional<std::is_array<To>::value,
                                                                std::is_convertible<
                                                                    typename std::remove_extent<From>::type (*)[],
                                                                    typename std::remove_extent<To>::type (*)[]>,
                                                                std::is_convertible<From*, To*>>::type::value> {};

    template <class U>
    void delete_owner(void* p) noexcept {
        delete static_cast<UnqPtr<U>*>(p);
    }
}

template <class T>
class WeakPtr;

// T may be an object type or an unbounded array type.
template <class T>
class ShrdPtr {
    template <class>
    friend class ShrdPtr;
    template <class>
    friend class WeakPtr;
    using Element = typename std::remove_extent<T>::type;
    ControlBlock* control = nullptr;
    Element* pointer = nullptr;

    void release() noexcept {
        ControlBlock* old = control;
        control = nullptr;
        pointer = nullptr;
        if (old && --old->count == 0) {
            // Detach before destruction: the object may contain WeakPtr members.
            void* owner = old->owner;
            old->owner = nullptr;
            old->deleter(owner);
            if (--old->weakCount == 0) delete old;
        }
    }

public:
    ShrdPtr() noexcept = default;

    explicit ShrdPtr(UnqPtr<T>* p) : ShrdPtr() {
        if (p) {
            try {
                control = new ControlBlock(p, &shrd_detail::delete_owner<T>);
            }
            catch (...) {
                delete p;
                throw;
            }
            pointer = p->get();
        }
    }

    ShrdPtr(const ShrdPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control) ++control->count;
    }

    template <class U, typename std::enable_if<shrd_detail::compatible<U, T>::value, int>::type = 0>
    ShrdPtr(const ShrdPtr<U>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control) ++control->count;
    }

    ~ShrdPtr() {
        release();
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        ShrdPtr(other).swap(*this);
        return *this;
    }

    template <class U, typename std::enable_if<shrd_detail::compatible<U, T>::value, int>::type = 0>
    ShrdPtr& operator=(const ShrdPtr<U>& other) noexcept {
        ShrdPtr(other).swap(*this);
        return *this;
    }

    void swap(ShrdPtr& other) noexcept {
        std::swap(control, other.control);
        std::swap(pointer, other.pointer);
    }

    Element* get() const noexcept {
        return pointer;
    }

    std::size_t use_count() const noexcept {
        return control ? control->count : 0;
    }

    template <class Q = T, typename std::enable_if<std::is_array<Q>::value, int>::type = 0>
    Element& operator[](std::size_t index) const {
        return pointer[index];
    }
};
#endif
