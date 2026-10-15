#include <iostream>
#include "shrd_ptr.h"

class Base
{
public:
    int value;

    Base(int number) : value(number) {}
};

class Derived : public Base
{
public:
    Derived(int number) : Base(number)
    {
        std::cout << "Объект создан\n";
    }

    ~Derived()
    {
        std::cout << "Объект удалён\n";
    }
};

int main()
{
    {
        ShrdPtr<Derived> first(
            new UnqPtr<Derived>(new Derived(20))
        );

        {
            ShrdPtr<Derived> second(first);

            second.get()->value = 50;

            std::cout << "Через first: "
                      << first.get()->value << '\n';

            ShrdPtr<Base> third;
            third = second;

            std::cout << "Через Base: "
                      << third.get()->value << '\n';
        }

        std::cout << "Другие дескрипторы уничтожены\n";

        std::cout << "first ещё работает: "
                  << first.get()->value << '\n';
    }

    std::cout << "Все дескрипторы уничтожены\n";

    ShrdPtr<int> empty;

    std::cout << "Пустой указатель: "
              << (empty.get() == nullptr) << '\n';
}