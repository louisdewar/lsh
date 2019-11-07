//
// Created by Louis de Wardt on 07/11/2019.
//

#include "util.h"

int pow_int(int a, int n) {
    for(int count = 0; count < n; count++) {
        a *= a;
    }

    return a;
}

int pow_2(int n) {
    return 2 << (n - 1);
}