#include "assert_util.h"

void test() {
    int a = 2;
    SYLAR_ASSERT(a == 1);
}

int main() {
    test();
    return 0;
}