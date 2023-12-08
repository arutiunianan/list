#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const int DEAD_VALUE = 2003;
const int FREE_INDEX = -1 ;

typedef struct Elem
{

    int next;
    int prev;
    int val;
}Elem;

typedef struct List
{

    Elem* data;

    int tail;
    int head;
    int free;
    int size;
    int capacity;
    int sorted;

    int errors;
}List;

void ListCtor( List* list, int capacity );
void ListDtor( List* list );
void ListInsertAfter ( List* list, int index, int value );
void ListInsertBefore( List* list, int index, int value );
void ListPushBack ( List* list, int value );
void ListPushFront( List* list, int value );
int ListRemove( List* list, int index );
int ListPopBack( List* list );
int ListPopFront( List* list );
int ListValueIndex(List* list, int value);
int ListVerify( List* list );
void ListLinearize( List* list );
int ListDump(List *list);


typedef enum Status
{

    NoErrors = 0,
    ListPtrError = 1 << 1,
    DataPtrError = 1 << 2,
    CapasityIsNegative = 1 << 3,
    HeadBiggerThanCapasity = 1 << 4,
    TailBiggerThanCapasity = 1 << 5,
    SizeBiggerThanCapasity  = 1 << 6,
    FreeBiggerThanCapasity  = 1 << 7,
    ZeroIndexError = 1 << 8,
    SortedError = 1 << 9,
    HeadError = 1 << 10,
    TailError = 1 << 11,
    DataError = 1 << 12,
    IndexesError = 1 << 13,
    SizeError = 1 << 14,
    InsertIndexError = 1 << 15,
    ResizeError = 1 << 16,
}Status;

typedef enum Resize
{

    Increase = 1,
    Decrease = -1,
}Resize;

#endif // #define LIST_H_