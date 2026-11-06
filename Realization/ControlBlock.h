#ifndef CONTROL_BLOCK_H
#define CONTROL_BLOCK_H

#include "unq_ptr.h"
#include <cstddef>
#include <memory>
#include <type_traits>

template<class T> class TypedControlBlock;

// Общая часть блока не зависит от типа объекта, поэтому её могут разделять
// указатели на производный и базовый классы.
class ControlBlock {
    std::size_t strongCount = 1;
    // Пока объект жив, дополнительная слабая ссылка защищает блок при его удалении.
    std::size_t weakCount = 1;

protected:
    ControlBlock() = default;
    virtual ~ControlBlock() = default;
    virtual void destroyOwner() noexcept = 0;

public:
    ControlBlock(const ControlBlock&) = delete;
    ControlBlock& operator=(const ControlBlock&) = delete;

    template<class T>
    static ControlBlock* create(UnqPtr<T>* owner);

    template<class T>
    static ControlBlock* fromRaw(std::remove_extent_t<T>* pointer);

    void incStrong() noexcept { ++strongCount; }
    void incWeak() noexcept { ++weakCount; }

    void decWeak() noexcept {
        if (--weakCount == 0) delete this;
    }

    void decStrong() noexcept {
        if (--strongCount == 0) {
            destroyOwner();
            decWeak();
        }
    }

    std::size_t useCount() const noexcept { return strongCount; }
};

// Типизированная часть знает, как удалить исходного владельца объекта или массива.
template<class T>
class TypedControlBlock final : public ControlBlock {
    UnqPtr<T>* owner;

    void destroyOwner() noexcept override {
        delete owner;
        owner = nullptr;
    }

public:
    explicit TypedControlBlock(UnqPtr<T>* owner) noexcept : owner(owner) {}
};

template<class T>
ControlBlock* ControlBlock::create(UnqPtr<T>* owner) {
    if (owner == nullptr) return nullptr;

    // При ошибке выделения памяти владелец и его объект будут удалены.
    std::unique_ptr<UnqPtr<T>> guard(owner);
    ControlBlock* block = new TypedControlBlock<T>(owner);
    guard.release();
    return block;
}

template<class T>
ControlBlock* ControlBlock::fromRaw(std::remove_extent_t<T>* pointer) {
    if (pointer == nullptr) return nullptr;

    std::unique_ptr<T> guard(pointer);
    UnqPtr<T>* owner = new UnqPtr<T>(pointer);
    guard.release();
    return create(owner);
}

#endif
