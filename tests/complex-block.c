#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

void foo(int x) {
    patch_point(&ppl, "pp") {
        int i = 0;
        while (i < 2) {
            printf("%d\n", i++);
        }
        if (x == 1) {
            printf("x == 1\n");
        }
    } else {
        printf("disabled\n");
    }
}

int main() {
    foo(1);
    patch_point_disable(&ppl, "pp");
    foo(1);
}

/*
 * check-name: patch point with big block
 * check-output-start
0
1
x == 1
disabled
 * check-output-end
 */
