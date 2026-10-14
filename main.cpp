#include <iostream>
#include "unq_ptr.h"

using namespace std;
int main() {
    UnqPtr<int> pointer(new int(210));
    UnqPtr<int> pointer2(pointer);

    *pointer2.get() = 42;
    cout << *pointer.get() << "  " << *pointer2.get();


}
