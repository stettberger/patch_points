#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

void foo() {
    patch_point(&ppl, "pp") {
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
        printf("enabled\n");
    } else {
        printf("disabled\n");
    }
}

int main() {
    foo();
    patch_point_disable(&ppl, "pp");
    foo();
}

/*
 * check-name: patch point with big block
 * check-output-start
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
enabled
disabled
 * check-output-end
 */
