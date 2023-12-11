#include "list.h"

#define PREV( index ) list->data[index].prev
#define NEXT( index ) list->data[index].next
#define VAL( index ) list->data[index].val


static Elem* ListResize( List *list, Resize mode )
{
    list->errors = ListVerify(list);

    if( list->errors != NoErrors )
        return NULL;
    
    int new_capacity = 0;
    
    if( mode == Increase )
    {

        new_capacity = list->capacity * 2;

        Elem* new_data = ( Elem* )realloc( list->data, ( 1 + new_capacity ) * sizeof( Elem ) );
        if( new_data == NULL )
            return NULL;

        int new_free = list->capacity ;

        int elem = list->free;
        while( new_data[elem].next != 0 )
        {

            elem =  NEXT( elem );
        }
        if( elem == list->free )
            list->free = new_free;
        else
             NEXT( elem ) = new_free;

        for( int i = new_free; i < new_free + list->capacity; i++ )
        {

            new_data[i].prev  = FREE_INDEX;
            new_data[i].next  = i + 1;
            new_data[i].val = DEAD_VALUE;
        }

        new_data[new_capacity].next = 0;
        list->capacity = new_capacity; 

        return new_data;
    }
    else if( mode == Decrease )
    {

        new_capacity = list->capacity / 2;
        assert(list->size <= new_capacity);

        if( new_capacity < 4 ) 
            new_capacity = 4;

        if( new_capacity == list->capacity )
            return list->data;

        if( list->sorted == 0 
            ||
            list->sorted == 1 && list->tail > new_capacity )
        {   
            ListLinearize( list );
            if( list->errors != NoErrors )
                return NULL;
        }

        list->free = 0;
        for( int i = new_capacity; i >= 1; i-- )
        {

            if( PREV( i ) == FREE_INDEX )
            {

                if( list->free == 0 )
                {

                    list->free = i;
                     NEXT( i )= 0;
                }
                else
                {

                    NEXT( i ) = list->free;
                    list->free = i;
                }
            }
        }

        Elem* new_data = ( Elem* )realloc( list->data, ( 1 + new_capacity ) * sizeof( Elem ) );
        if( new_data == NULL )
        {

            list->errors |= DataPtrError;
            return NULL;
        }

        list->capacity = new_capacity;

        return new_data;
    }
    else
    {

        list->errors |= ResizeError;
        return NULL;
    }

    return NULL;
}

void ListCtor( List* list, int capacity )
{

    list->errors = NoErrors;

    if ( !list ) 
        list->errors |= ListPtrError;

    if ( capacity <= 0 ) 
        list->errors |= CapasityIsNegative;

    list->data = ( Elem* )calloc( capacity + 1, sizeof( Elem ) );
    list->capacity = capacity;
    list->head = 1;
    list->tail = 1;
    list->free = 1;
    list->sorted = 1;
    list->size = 0;

    for ( int i = 1; i < capacity + 1; i++ )
    {
        NEXT( i )  = i + 1;
        PREV( i )  = FREE_INDEX;
        VAL( i ) = DEAD_VALUE;
    }
}

void ListDtor( List *list )
{

    if( ListVerify( list ) == NoErrors )

        for ( int i = 0; i < list->capacity + 1; ++i )
        {

            NEXT( i ) = 0xDED2410;
            PREV( i ) = 0xDED2410;
            VAL( i ) = DEAD_VALUE;
        }  

    free(list->data);
    list->data = NULL;
    list->capacity = 0xDEAD;
    list->free = 0xDEAD;
    list->tail = 0xDEAD;
    list->head = 0xDEAD;
    list->size = 0xDEAD;
    list->sorted = 0xDEAD;
    list->errors = ( Status ) 0xD70D;

    return;
}

void ListInsertAfter ( List* list, int index, int value )
{
    list->errors = ListVerify( list );
    
    
    if( list->errors != NoErrors )
        return;
    
    if( index == 0 
        || 
        index > list->capacity )
    {

        list->errors = InsertIndexError;
        return;
    }
    
    if( index != list->tail )
        list->sorted = 0;

    if( list->free >= list->capacity )
    {   
        Elem* ptr = ListResize( list, Increase );
        if( ptr == NULL )
        {

            list->errors |= DataPtrError;
            return;
        }
        list->data = ptr;
    }
    assert(list->free != 0);

    if( list->head == index && list->head == list->tail && PREV( list->head ) == FREE_INDEX )
    {
        assert( list->free == list->tail );

        list->free = NEXT( list->free );

        VAL( list->tail ) = value;
        NEXT( list->tail )  = 0;
        PREV( list->tail )  = 0;
    }
    else if( index == list->tail )
    {
        
        PREV( list->free )  = list->tail;
        VAL( list->free ) = value;

        int index_new = list->free;
        NEXT( list->tail )  = index_new;
        list->tail = index_new;
        list->free = NEXT( index_new );
        NEXT( index_new ) = 0;
    }
    else
    {

        int new_free = NEXT( list->free );

        NEXT( list->free ) = NEXT( index );
        PREV( list->free ) = index;
        VAL( list->free ) = value;
        
        NEXT( PREV( list->free ) ) = list->free;
        PREV( NEXT( list->free ) ) = list->free;

        list->free = new_free;
        
    }
    list->size++;

    list->errors = ListVerify(list);
}

void ListInsertBefore( List* list, int index, int value )
{

    list->errors = ListVerify( list );

    if( list->errors != NoErrors )
        return;

    if( index == 0 
        || 
        index > list->capacity 
        ||
        PREV( index ) == FREE_INDEX && ( list->size != 0 || index != list->tail ) )
    {

        list->errors = InsertIndexError;
        return;
    }
    
    if( list->head == list->tail )
        return ListInsertAfter( list, index, value );
    
    if( list->free == 0 )
    {
        Elem* ptr = ListResize( list, Increase );
        if( ptr == NULL )
        {

            list->errors |= DataPtrError;
            return;
        }
        list->data = ptr;
    }
    assert(list->free != 0);

    if( index == list->head )
    {

        int new_free = NEXT( list->free );

        PREV( list->head ) = list->free;
        NEXT( list->free )  = list->head;
        PREV( list->free )  = 0;
        VAL( list->free ) = value;
        list->head = list->free;
        list->free = new_free;

        list->size++;
    }
    else
    {
        return ListInsertAfter(list, PREV( index ), value);
    }

    list->errors = ListVerify(list);
}

void ListPushBack ( List* list, int value )
{

    return ListInsertAfter( list, list->tail, value );
}

void ListPushFront( List* list, int value )
{

    return ListInsertBefore( list, list->head, value );
}

int ListRemove( List* list, int index )
{

    if( list->errors != NoErrors )
        return DEAD_VALUE;
    
    if( index == 0 || index > list->capacity + 1 || PREV( index ) == FREE_INDEX )
        return DEAD_VALUE;
    
    list->size--;

    int value = DEAD_VALUE;

    if( index == list->head )
    {
        if( index == list->tail )
            if( PREV( index ) == FREE_INDEX )
            {

                assert(++list->size == 0);
                return DEAD_VALUE;
            }
            else
            {

                value = VAL( index );
                
                PREV( index )  = FREE_INDEX;
                NEXT( index )  = list->free;
                VAL( index ) = DEAD_VALUE;

                list->free = index;
            }
        else
        {
            value = VAL( list->head );
            int new_head = NEXT( list->head );

            PREV( new_head )  = 0;
                
            PREV( list->head )  = FREE_INDEX;
            NEXT( list->head )  = list->free;
            VAL( list->head ) = DEAD_VALUE;
            list->free  = list->head;

            list->head = new_head;
        }
    }
    else if( index == list->tail )
    {
        value = VAL( list->tail );

        int new_tail = PREV( list->tail );

        NEXT( new_tail ) = 0;
        
        PREV( list->tail )  = FREE_INDEX;
        NEXT( list->tail )  = list->free;
        VAL( list->tail ) = DEAD_VALUE;
        list->free = list->tail;

        list->tail = new_tail;
    }
    else
    {

        int next = NEXT( index );
        int prev = PREV( index );
    
        value = VAL( index );

        PREV( index )  = FREE_INDEX;
        NEXT( index )  = list->free;
        VAL( index ) = DEAD_VALUE;

        list->free = index;

        PREV( next ) = prev;
        NEXT( prev ) = next;
    }

    if (list->size <= list->capacity / 4)
    {

        Elem* ptr = ListResize( list, Decrease );
        if( ptr == NULL )
        {
            return DEAD_VALUE;
        }
        list->data = ptr;
    }

    return value;
}

int ListPopBack( List* list )
{

    return ListRemove( list, list->tail );
}

int ListPopFront( List* list )
{

    return ListRemove( list, list->head );
}

int ListValueIndex(List* list, int value)
{

    list->errors = ListVerify( list );

    if( list->errors != NoErrors )
        return FREE_INDEX;
    
    int elem = FREE_INDEX;
    while( elem != FREE_INDEX && VAL( elem ) != value )
        elem = NEXT( elem );
    
    return elem;
}

int ListVerify( List* list )
{

    int error = NoErrors;

    if( list->data == NULL )
        error |= DataPtrError;

    if( list->head > list->capacity )
        error |= HeadBiggerThanCapasity;

    if( list->tail  > list->capacity )
        error |= TailBiggerThanCapasity;

    if( list->size > list->capacity )
        error |= SizeBiggerThanCapasity;
    
    if( list->free   > list->capacity ) 
        error |= FreeBiggerThanCapasity;

    if( NEXT( 0 )  != 0 || PREV( 0 )  != 0 || VAL( 0 ) != 0 )
        error |= ZeroIndexError;
    
    if( list->sorted > 1 )
        error |= SortedError;

    if( error != NoErrors )
        return ( Status ) error;

    if( list->tail == list->head )
    {   
        if( !( NEXT( list->head ) == 0 && PREV( list->head ) == 0 && list->size == 1
              ||
              PREV( list->head ) == FREE_INDEX && VAL( list->head ) == DEAD_VALUE && list->size == 0 ) )
            error |= DataError;
        
        for( int i = 1; i <= list->capacity; i++ )
        {
            if( i != list->head )
            {
                if( VAL( i ) != DEAD_VALUE || PREV( i ) != FREE_INDEX )
                {
                    error |= DataError;
                    break;
                }
            }
        }
    }

    if( error != NoErrors )
        return ( Status ) error;
    
    
    if( list->head != list->tail && PREV( list->head ) != 0 )
        error |= HeadError;
    
    if( list->head != list->tail && NEXT( list->tail ) != 0 )
        error |= TailError;

    if( error != NoErrors )
        return ( Status ) error;
    
    for( int i = 1; i <= list->capacity; i++ )
    {

        if( NEXT( i )  >  list->capacity && i != list->capacity
            ||
            ( PREV( i )  >  list->capacity && PREV( i )  != FREE_INDEX )
            || 
            ( PREV( i )  == FREE_INDEX && VAL( i ) != DEAD_VALUE ) )
        {
            
            error |= IndexesError;
            break;
        }
    }

    if( error != NoErrors )
        return ( Status ) error;

    int size  = 0;
    int place = list->head;
    
    while( list->head != list->tail && size < list->capacity )
    {
        if( NEXT( place ) == 0 )
        {

            if( place != list->tail )
                error |= DataError;
            size++;
            break;
        }
        else if( PREV( NEXT( place ) ) != place
            ||
            PREV( place ) != 0 && NEXT( PREV( place ) ) != place )
        {

            error |= DataError;
            break;
        }

        place = NEXT( place );
        size++;
    }

    if( list->head == list->tail && PREV( list->head ) != FREE_INDEX )
        size++;

    place = list->free;

    while( size < list->capacity && place != 0 && place < list->capacity )
    {

        if( NEXT( place ) == 0 )
        {

            if( PREV( place ) != FREE_INDEX 
                ||
                VAL( place ) != DEAD_VALUE )
                error |= DataError;
            size++;
            break;
        }
        else if( PREV( place )  != FREE_INDEX 
                 ||
                 VAL( place ) != DEAD_VALUE )
        {

            error |= DataError;
            break;
        }
        
        place = NEXT( place );
        size++;
    }
    
    return ( Status ) error;
}

void ListLinearize( List* list )
{
    list->errors = ListVerify(list);
    if( list->errors != NoErrors)
        return;

    if( list->sorted == 1 && PREV( 1 ) != FREE_INDEX )
    {

        list->errors = NoErrors;
        return;
    }
        
    Elem* new_data = ( Elem* )calloc( 1 + list->capacity, sizeof( Elem ) );
    if (new_data == NULL)
    {

        list->errors = DataPtrError;
        return;
    }

    new_data[0].prev  = 0;
    new_data[0].next  = 0;
    new_data[0].val = 0;

    size_t cur_ptr = list->head;
    if( list->size == 0 )
        cur_ptr = 0;

    list->free = 0;
    for( size_t i = 1; i <= list->capacity; i++ )
    {
        if (cur_ptr != 0)
        {
            new_data[i].prev  = (i == 1 ? 0 : i - 1);
            new_data[i].next  = ( i == list->size ? 0 : i + 1);
            new_data[i].val = VAL( cur_ptr );
        }
        else
        {

            if (list->free == 0) 
                list->free = i;
            
            new_data[i].prev  = FREE_INDEX;
            new_data[i].next  = ( i == list->capacity ? 0 : i + 1 );
            new_data[i].val = DEAD_VALUE;
        }

        cur_ptr = NEXT( cur_ptr );
    }

    free(list->data);

    list->sorted = 1;
    list->head = 1;
    list->tail = (list->size == 0 ? 1 : list->size);
    list->data = new_data;
}

static const char *ColorPicker( List *list, int index)
{
    const char *colors[] =
    {
        "#E6A8D7", // [0] pink
        "#42AAFF", // [1] cyan
        "#1CD3A2", // [2] green
        "#FF7538", // [3] orange
        "#E34234", // [4] red
        "#5035DE", // [5] (almost) royal blue
        "#EED202", // [6] YELLOW WARNING
    };

    if ( ListVerify(list) )
        return colors[6];

    if (index <= list->capacity + 1 && PREV( index ) == FREE_INDEX)
        return colors[1];
    else
    if (index == 0)
        return colors[5];
    else
    if (index == list->tail)
        return colors[3];
    else
    if (index == list->head)
        return colors[2];
    else
    if (index > list->capacity + 1)
        return colors[4];
    else
        return colors[0];
}

int ListDump(List *list)
{
    if ( ListVerify(list) )
    {
        return ListVerify(list);
    }

    FILE* dump_file = fopen("dump.gv", "w");

    fputs("digraph structs {\n", dump_file);

    fputs("    node [color=black, shape=box, style=\"rounded, filled\"];\n", dump_file);
    
    fputs("    rankdir=LR;\n", dump_file);
  
    fprintf(dump_file, "    head [fillcolor=\"%s\", "
                       "    label=\"HEAD = %lu\"];\n",
                       ColorPicker(list, list->head), list->head);

    fprintf(dump_file, "    tail  [fillcolor=\"%s\", "
                       "    label=\"TAIL = %lu\"];\n",
                       ColorPicker(list, list->tail), list->tail);

    fprintf(dump_file, "    free  [fillcolor=\"%s\", "
                       "    label=\"FREE = %lu\"];\n",
                       ColorPicker(list, list->free), list->free);

    fprintf(dump_file, "    sorted [fillcolor=\"%s\","
                       "    label=\"SORTED = %d\"];\n",
                       ColorPicker(list, FREE_INDEX / 2), list->sorted);

    fprintf(dump_file, "    size   [fillcolor=\"%s\","
                       "    label=\"SIZE = %d\"];\n",
                       ColorPicker(list, FREE_INDEX / 2), list->size);                     

    fputs("    node [color=black, shape=record, style=\"rounded, filled\"];\n", dump_file);
    fputs("\n", dump_file);
    fputs("    edge [style=invis, constraint=true];\n", dump_file);

    for (size_t index = 0; index <= list->capacity - 1; ++index)
    {
        fprintf( dump_file, "    node%lu [fillcolor=\"%s\","
                "label=\" %lu | { <p> %d | %d | <n> %lu}\"];\n",
                index, ColorPicker(list, index), index,
                (PREV( index ) == FREE_INDEX) ? -1 : PREV( index ),
                VAL( index ), NEXT( index ) );
        
        if ( index > 0 ) fprintf( dump_file, "    node%lu -> node%lu;\n", index - 1, index );
    }

    fputs( "\n    edge [style=solid, constraint=false];\n", dump_file );

    for ( size_t index = 1; index <= list->capacity; ++index )
    {
        if ( NEXT( index ) != 0 )
            fprintf(dump_file, "    node%lu: <n> -> node%lu;\n", index, NEXT( index ));
        if ( PREV( index ) != FREE_INDEX && PREV( index ) != 0 )
            fprintf(dump_file, "    node%lu: <p> -> node%lu;\n", index, PREV( index ));
        fputs( "\n", dump_file );
    }

    fputs("\n    edge [style=bold, constraint=false];\n", dump_file);

    fprintf(dump_file, "    head -> node%lu; \n", list->head);
    fprintf(dump_file, "    tail  -> node%lu; \n", list->tail );
    fprintf(dump_file, "    free  -> node%lu; \n", list->free );

    fputs("}\n", dump_file);

    fclose(dump_file);

    return NoErrors;
}