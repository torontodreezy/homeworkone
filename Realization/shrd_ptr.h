#ifndef SHRD_PTR_H
#define SHRD_PTR_H

#include "unq_ptr.h"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

// Общий блок владения. Адрес для доступа хранится отдельно в ShrdPtr.
struct ControlBlock {
    using Deleter = void (*)(void*);

    std::size_t count = 1;
    // Одна служебная слабая ссылка существует, пока есть сильные владельцы.
    std::size_t weakCount = 1;
    void* owner;
    Deleter deleter;

    ControlBlock(void* owner, Deleter deleter) noexcept
        : owner(owner), deleter(deleter) {}

    template<class U>
    static void deleteOwner(void* owner) {
        // Деструктор UnqPtr удаляет объект или массив, затем удаляется обёртка.
        delete static_cast<UnqPtr<U>*>(owner);
    }

    template<class U>
    static ControlBlock* create(UnqPtr<U>* owner) {
        if (owner == nullptr) return nullptr;
        try {
            return new ControlBlock(owner, deleteOwner<U>);
        } catch (...) {
            delete owner;
            throw;
        }
    }

    template<class U>
    static ControlBlock* fromRaw(typename std::remove_extent_t<U>* pointer) {
        if (pointer == nullptr) return nullptr;
        // Защита объекта, если выделение памяти под обёртку не удастся.
        std::unique_ptr<U> guard(pointer);
        auto* owner = new UnqPtr<U>(pointer);
        guard.release();
        return create(owner);
    }

    void addStrong() noexcept { ++count; }
    void addWeak() noexcept { ++weakCount; }

    void releaseWeak() noexcept {
        if (--weakCount == 0) delete this;
    }

    void releaseStrong() noexcept {
        if (--count == 0) {
            void* oldOwner = owner;
            owner = nullptr;
            // Служебная ссылка защищает блок при удалении WeakPtr внутри объекта.
            deleter(oldOwner);
            releaseWeak();
        }
    }
};

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
        if (old != nullptr) old->releaseStrong();
    }

public:
    ShrdPtr() noexcept = default;
    ShrdPtr(std::nullptr_t) noexcept {}

    explicit ShrdPtr(UnqPtr<T>* owner)
        : control(ControlBlock::create(owner)),
          pointer(owner == nullptr ? nullptr : owner->get()) {}

    ShrdPtr(const ShrdPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->addStrong();
    }

    // Разрешаем только безопасное преобразование адресов.
    template<class U, class = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr(const ShrdPtr<U>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->addStrong();
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
        return control == nullptr ? 0 : control->count;
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
        if (old != nullptr) old->releaseStrong();
    }

public:
    ShrdPtr() noexcept = default;
    ShrdPtr(std::nullptr_t) noexcept {}

    explicit ShrdPtr(UnqPtr<T[]>* owner)
        : control(ControlBlock::create(owner)),
          pointer(owner == nullptr ? nullptr : owner->get()) {}

    ShrdPtr(const ShrdPtr& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->addStrong();
    }

    // Разрешаем только безопасное преобразование адресов.
    template<class U, class = std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>>>
    ShrdPtr(const ShrdPtr<U[]>& other) noexcept
        : control(other.control), pointer(other.pointer) {
        if (control != nullptr) control->addStrong();
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
        return control == nullptr ? 0 : control->count;
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
