#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

int main() {
    int i;
    for (i = 0; i< 100; i++) {
        patch_point(&ppl, "pp") {
            printf("enabled\n");
        }
        patch_point_disable(&ppl, "pp");
    }
}

/*
 * check-name: patch point with biggerloop
 * check-output-start
enabled
 * check-output-end
 */
