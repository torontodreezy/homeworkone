#include "TestUnqPtr.h"
#include "TestShrdPtr.h"

void checkAllDeleted()
{
    tests::check(
        tests::Tracked::alive == 0,
        "Остались Tracked"
    );

    tests::check(
        tests::Derived::alive == 0,
        "Остались Derived"
    );

    tests::check(
        tests::ArrayTracked::alive == 0,
        "Остались элементы массива"
    );

    tests::check(
        tests::UnqDerived::alive == 0,
        "Остались UnqDerived"
    );
}

int main()
{
    tests::runUnqTests();
    tests::runShrdTests();

    tests::run(
        "Все отслеживаемые объекты удалены",
        checkAllDeleted
    );

    return tests::summary();
}