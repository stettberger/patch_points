#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

void
foo() {
    patch_point(&ppl, "pp1") {
        printf("pp1\n");
    } else patch_point(&ppl, "pp2") {
        printf("pp2\n");
    } else {
        printf("disabled\n");
    }
}

int main() {
    foo();
    patch_point_disable(&ppl, "pp1");
    foo();
    patch_point_disable(&ppl, "pp2");
    foo();
    patch_point_enable(&ppl, "pp1");
    foo();
}

/*
 * check-name: if else-if else chain
 * check-output-start
pp1
pp2
disabled
pp1
 * check-output-end
 */
