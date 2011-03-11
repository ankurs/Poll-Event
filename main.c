#include<stdio.h>
#include "hash_table/hashtable.h"

int main()
{
    hash_table_t *table = hash_table_new(MODE_COPY);
    int i = 1;
    int val = 100;
    int val2 = 200;
    int j = 2;
    int x =0;
    for (x=0;x<100;x++)
    {
        HT_ADD(table, &j, &val);
        //hash_table_add(table, &j, i, (void *) &val, sizeof(int));
        val++;
        j++;
    }
    hash_table_add(table, &j, i, (void *) &val2, 1);
    hash_table_remove(table, &j, i);
    if (hash_table_has_key(table, &j, i))
    {
        printf("Key found\n");
    }
    else
    {
        printf("Key NOT found\n");
    }
    val = -100;
    val2 = -200;
    int *value = (int* ) hash_table_lookup(table, &j, i);
    if (value)
        printf("value is %d\n", *value);
    else
        printf("*value is %p\n", value);
    void** keys;
    size_t num = hash_table_get_keys(table, keys);
    printf("found %d keys\n", (int)num);
    printf("%d - %d\n", (int)sizeof(value), (int)sizeof(*value));
    return 0;
}
