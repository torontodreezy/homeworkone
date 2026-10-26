#include "Tests/TestUnqPtr.h"
#include "Tests/TestShrdPtr.h"
#include "Tests/TestWeakPtr.h"
#include "Tests/TestStandardComparison.h"

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
    tests::check(tests::WeakNode::alive == 0, "Остались WeakNode");
    tests::check(tests::WeakArrayNode::alive == 0, "Остались WeakArrayNode");

}

int main()
{
    tests::runUnqTests();
    tests::runShrdTests();
    tests::runWeakTests();
    tests::runStandardComparisonTests();

    tests::run(
        "Все отслеживаемые объекты удалены",
        checkAllDeleted
    );

    return tests::summary();
}
