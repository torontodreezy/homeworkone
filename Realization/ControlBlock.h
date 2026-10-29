#ifndef CONTROL_BLOCK_H
#define CONTROL_BLOCK_H

#include "unq_ptr.h"
#include <cstddef>
#include <memory>
#include <type_traits>

// Один общий блок для всех сильных и слабых указателей на один объект.
class ControlBlock {
private:
    // Тип функции: принимает адрес, ничего не возвращает.
    using DeleteFunction = void(void*);

    void* ownerPointer;
    DeleteFunction* deleteFunction;
    std::size_t strongCount = 1;

    std::size_t weakCount = 1;

    ControlBlock(void* owner, DeleteFunction* destroy) noexcept
        : ownerPointer(owner), deleteFunction(destroy) {}

    template <class T>
    static void deleteOwner(void* address) {
        UnqPtr<T>* owner = static_cast<UnqPtr<T>*>(address);
        // Удаляем обёртку; её деструктор удаляет объект или массив.
        delete owner;
    }

public:

    ControlBlock(const ControlBlock&) = delete;
    ControlBlock& operator=(const ControlBlock&) = delete;

    template <class T>
    static ControlBlock* create(UnqPtr<T>* owner) {
        if (owner == nullptr) {
            return nullptr;
        }
        try {
            return new ControlBlock(owner, deleteOwner<T>);
        }
        catch (...) {
            delete owner;
            throw;
        }
    }


    template <class T>
    static ControlBlock* fromRaw(std::remove_extent_t<T>* pointer) {
        if (pointer == nullptr) {
            return nullptr;
        }
        // Если new UnqPtr выбросит исключение, guard удалит переданные данные.
        std::unique_ptr<T> guard(pointer);
        UnqPtr<T>* owner = new UnqPtr<T>(pointer);
        guard.release();
        // Теперь за очистку при исключении отвечает create.
        return create(owner);
    }

    void incStrong() noexcept {
        ++strongCount;
    }

    void incWeak() noexcept {
        ++weakCount;
    }

    void decWeak() noexcept {
        --weakCount;
        if (weakCount == 0) {
            delete this;
        }
    }

    void decStrong() noexcept {
        --strongCount;
        if (strongCount == 0) {
            void* oldOwner = ownerPointer;
            ownerPointer = nullptr;
            // Служебная ссылка защищает блок от удаления WeakPtr внутри объекта.
            deleteFunction(oldOwner);
            // Объект удалён: снимаем служебную слабую ссылку.
            decWeak();
        }
    }

    std::size_t useCount() const noexcept {
        return strongCount;
    }

    bool expired() const noexcept {
        return strongCount == 0;
    }
};

#endif
