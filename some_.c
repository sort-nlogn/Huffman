#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *out = fopen("out.bin", "rb");
    char c = 0;
    while(!feof(out)){
        c = fgetc(out);
        printf("%d\n", c);
    }
    return 0;
}