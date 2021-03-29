#include<stdio.h>

int main() {
    printf("Hello World!\n");

    char username[50];
    char institue[50];
    if(scanf("%s %s", username, institue)) {
        printf("username=%s\n", username);
    }
    
    return 0;
}