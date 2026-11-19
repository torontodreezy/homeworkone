#ifndef CONTROL_BLOCK_H
#define CONTROL_BLOCK_H

#include "unq_ptr.h"
#include <cstddef>

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
    static ControlBlock* create(UnqPtr<T>&& owner);

    void incStrong() noexcept { ++strongCount; }
    bool tryIncStrong() noexcept {
        if (strongCount == 0) return false;
        ++strongCount;
        return true;
    }
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
    UnqPtr<T> owner;

    void destroyOwner() noexcept override {
        owner.reset();
    }

public:
    explicit TypedControlBlock(UnqPtr<T>&& source) noexcept
        : owner(std::move(source)) {}
};

template<class T>
ControlBlock* ControlBlock::create(UnqPtr<T>&& owner) {
    // Если память не выделится, объект останется во владении исходного UnqPtr.
    return new TypedControlBlock<T>(std::move(owner));
}

#endif
