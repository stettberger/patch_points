#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

int main() {
    int i;
    for (i = 0; i< 10; i++) {
        patch_point(&ppl, "pp") {
            printf("enabled %d %x\n", i, i);
        } else {
            printf("disabled %d %x\n", i, i);
        }
        patch_point_set(&ppl, "pp", i % 2);
    }
}

/*
 * check-name: patch point with small loop
 * check-output-start
enabled 0 0
disabled 1 1
enabled 2 2
disabled 3 3
enabled 4 4
disabled 5 5
enabled 6 6
disabled 7 7
enabled 8 8
disabled 9 9
 * check-output-end
 */
