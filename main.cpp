#include "list.h"

int test( List* list )
{
    if( !list )
        return ListPtrError;

    int value = 0;
    ListPushBack(list, 1);

    for( int i = 2; i < 12; ++i )
    
        if( list->errors == NoErrors )
            ListInsertAfter( list, i - 1, i);

        else return list->errors;
    
    //printf( "%d ",list->size );
    /*list->data[2].val = 2003;
    list->data[2].prev = -1;
    list->data[2].next = 0;
    list->free = 2;*/
    //print( list );
    
    list->errors = ListVerify( list );

    if( list->errors == NoErrors )
        for (int i = 0; i < 4; ++i)
        {
            ListPopFront( list );
        }
    else return list->errors;

    ListDump( list );

    list->errors = ListVerify( list );

    if( list->errors == NoErrors )
        for (int i = 0; i < 5; ++i)
        {
            value = ListPopBack( list );
        }
    else return list->errors;

    list->errors = ListVerify( list );

    if( list->errors == NoErrors )
        for (int i = 0; i < 3; ++i)
        {
            value = ListPopFront( list );
            printf( " " );
        }
    else return list->errors;

    list->errors = ListVerify( list );

    ListDtor( list );
}

int main()
{

    List list = {};
    ListCtor( &list, 4 );
    /*
    printf("\n");
    for( int i = 0; i < 12; ++i )
    {
        printf("   %dы%d %d %d %dы\n",i,list.data[i].next,list.data[i].prev,list.data[i].val );
    }*/


    test( &list );
}