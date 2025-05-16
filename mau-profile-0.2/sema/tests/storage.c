#include <stdlib.h>
#include <stdio.h>

typedef unsigned long uint64_t;

typedef struct {
    uint64_t key;
    uint64_t value;
} Data;

Data Map[100];
uint64_t Size = 0;

uint64_t Get(uint64_t key)
{
    for (int i = 0; i < Size; ++i) {
        if (Map[i].key == key)  return Map[i].value;
    }
    return 0;
}

void Set(uint64_t key, uint64_t value)
{   
    int i = 0;
    for (i = 0; i < Size; ++i) {
        if (Map[i].key == key)  break;
    }
    Map[i].key = key;
    Map[i].value = value;
    ++Size;
}

int main()
{
    Set(10, 12);
    Set(10, 0);
    Set(1, 9);
    printf("%d\n", Get(10));
    printf("%d\n", Get(1));
    return 0;
}