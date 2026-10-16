#ifndef TEST_OBJECTS_H
#define TEST_OBJECTS_H

#include <stdexcept>

namespace tests
{
    struct Tracked
    {
        inline static int alive = 0;
        inline static int destroyed = 0;
        inline static bool failCopy = false;

        int value;

        explicit Tracked(int number) : value(number)
        {
            ++alive;
        }

        Tracked(const Tracked& other) : value(other.value)
        {
            if (failCopy)
            {
                throw std::runtime_error(
                    "Ошибка копирования для теста"
                );
            }

            ++alive;
        }

        ~Tracked()
        {
            --alive;
            ++destroyed;
        }
    };

    struct Base
    {
        int value = 10;
    };

    struct Derived : public Base
    {
        inline static int alive = 0;

        int extra = 20;

        Derived()
        {
            ++alive;
        }

        ~Derived()
        {
            --alive;
        }
    };

    struct Left
    {
        int left = 1;

        virtual ~Left() {}
    };

    struct Multiple : public Left, public Base
    {
    };

    struct Root
    {
        int root = 7;
    };

    struct Middle : public Root
    {
    };

    struct Leaf : public Middle
    {
    };

    // Элементы массива.
    struct ArrayTracked
    {
        inline static int alive = 0;
        inline static int destroyed = 0;

        int value = 0;

        ArrayTracked()
        {
            ++alive;
        }

        ArrayTracked(const ArrayTracked&) = delete;
        ArrayTracked& operator=(const ArrayTracked&) = delete;

        ~ArrayTracked()
        {
            --alive;
            ++destroyed;
        }
    };

    // Для проверки перемещения UnqPtr к базовому типу.
    struct UnqBase
    {
        int value = 10;

        virtual ~UnqBase() {}
    };

    struct UnqDerived : public UnqBase
    {
        inline static int alive = 0;

        UnqDerived()
        {
            ++alive;
        }

        ~UnqDerived() override
        {
            --alive;
        }
    };
}

#endif