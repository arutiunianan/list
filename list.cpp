#include "list.h"

#define PREV( index ) list->data[index].prev;
#define NEXT( index ) list->data[index].next;
#define VAL( index ) list->data[index].val;


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

            elem = list->data[elem].next;
        }
        if( elem == list->free )
            list->free = new_free;
        else
            new_data[elem].next = new_free;

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

            if( list->data[i].prev == FREE_INDEX )
            {

                if( list->free == 0 )
                {

                    list->free = i;
                    list->data[i].next = 0;
                }
                else
                {

                    list->data[i].next = list->free;
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
        list->data[i].next  = i + 1;
        list->data[i].prev  = FREE_INDEX;
        list->data[i].val = DEAD_VALUE;
    }
}

void ListDtor( List *list )
{

    if( ListVerify( list ) == NoErrors )

        for ( int i = 0; i < list->capacity + 1; ++i )
        {

            list->data[i].next = 0xDED2410;
            list->data[i].prev = 0xDED2410;
            list->data[i].val = DEAD_VALUE;
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

    if( list->head == index && list->head == list->tail && list->data[list->head].prev == FREE_INDEX )
    {
        assert( list->free == list->tail );

        list->free = list->data[list->free].next;

        list->data[list->tail].val = value;
        list->data[list->tail].next  = 0;
        list->data[list->tail].prev  = 0;
    }
    else if( index == list->tail )
    {
        
        list->data[list->free].prev  = list->tail;
        list->data[list->free].val = value;

        int index_new = list->free;
        list->data[list->tail].next  = index_new;
        list->tail = index_new;
        list->free = list->data[index_new].next;
        list->data[index_new].next = 0;
    }
    else
    {

        int new_free = list->data[list->free].next;

        list->data[list->free].next = list->data[index].next;
        list->data[list->free].prev = index;
        list->data[list->free].val = value;
        
        list->data[list->data[list->free].prev].next = list->free;
        list->data[list->data[list->free].next].prev = list->free;

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
        list->data[index].prev == FREE_INDEX && ( list->size != 0 || index != list->tail ) )
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

        int new_free = list->data[list->free].next;

        list->data[list->head].prev = list->free;
        list->data[list->free].next  = list->head;
        list->data[list->free].prev  = 0;
        list->data[list->free].val = value;
        list->head = list->free;
        list->free = new_free;

        list->size++;
    }
    else
    {
        return ListInsertAfter(list, list->data[index].prev, value);
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
    
    if( index == 0 || index > list->capacity + 1 || list->data[index].prev == FREE_INDEX )
        return DEAD_VALUE;
    
    list->size--;

    int value = DEAD_VALUE;

    if( index == list->head )
    {
        if( index == list->tail )
            if( list->data[index].prev == FREE_INDEX )
            {

                assert(++list->size == 0);
                return DEAD_VALUE;
            }
            else
            {

                value = list->data[index].val;
                
                list->data[index].prev  = FREE_INDEX;
                list->data[index].next  = list->free;
                list->data[index].val = DEAD_VALUE;

                list->free = index;
            }
        else
        {
            value = list->data[list->head].val;
            int new_head = list->data[list->head].next;

            list->data[new_head].prev  = 0;
                
            list->data[list->head].prev  = FREE_INDEX;
            list->data[list->head].next  = list->free;
            list->data[list->head].val = DEAD_VALUE;
            list->free  = list->head;

            list->head = new_head;
        }
    }
    else if( index == list->tail )
    {
        value = list->data[list->tail].val;

        int new_tail = list->data[list->tail].prev;

        list->data[new_tail].next = 0;
        
        list->data[list->tail].prev  = FREE_INDEX;
        list->data[list->tail].next  = list->free;
        list->data[list->tail].val = DEAD_VALUE;
        list->free = list->tail;

        list->tail = new_tail;
    }
    else
    {

        int next = list->data[index].next;
        int prev = list->data[index].prev;
    
        value = list->data[index].val;

        list->data[index].prev  = FREE_INDEX;
        list->data[index].next  = list->free;
        list->data[index].val = DEAD_VALUE;

        list->free = index;

        list->data[next].prev = prev;
        list->data[prev].next = next;
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
    while( elem != FREE_INDEX && list->data[elem].val != value )
        elem = list->data[elem].next;
    
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

    if( list->data[0].next  != 0 || list->data[0].prev  != 0 || list->data[0].val != 0 )
        error |= ZeroIndexError;
    
    if( list->sorted > 1 )
        error |= SortedError;

    if( error != NoErrors )
        return ( Status ) error;

    if( list->tail == list->head )
    {   
        if( !( list->data[list->head].next == 0 && list->data[list->head].prev == 0 && list->size == 1
              ||
              list->data[list->head].prev == FREE_INDEX && list->data[list->head].val == DEAD_VALUE && list->size == 0 ) )
            error |= DataError;
        
        for( int i = 1; i <= list->capacity; i++ )
        {
            if( i != list->head )
            {
                if( list->data[i].val != DEAD_VALUE || list->data[i].prev != FREE_INDEX )
                {
                    error |= DataError;
                    break;
                }
            }
        }
    }

    if( error != NoErrors )
        return ( Status ) error;
    
    
    if( list->head != list->tail && list->data[list->head].prev != 0 )
        error |= HeadError;
    
    if( list->head != list->tail && list->data[list->tail].next != 0 )
        error |= TailError;

    if( error != NoErrors )
        return ( Status ) error;
    
    for( int i = 1; i <= list->capacity; i++ )
    {

        if( list->data[i].next  >  list->capacity && i != list->capacity
            ||
            ( list->data[i].prev  >  list->capacity && list->data[i].prev  != FREE_INDEX )
            || 
            ( list->data[i].prev  == FREE_INDEX && list->data[i].val != DEAD_VALUE ) )
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
        if( list->data[place].next == 0 )
        {

            if( place != list->tail )
                error |= DataError;
            size++;
            break;
        }
        else if( list->data[list->data[place].next].prev != place
            ||
            list->data[place].prev != 0 && list->data[list->data[place].prev].next != place )
        {

            error |= DataError;
            break;
        }

        place = list->data[place].next;
        size++;
    }

    if( list->head == list->tail && list->data[list->head].prev != FREE_INDEX )
        size++;

    place = list->free;

    while( size < list->capacity && place != 0 && place < list->capacity )
    {

        if( list->data[place].next == 0 )
        {

            if( list->data[place].prev  != FREE_INDEX 
                ||
                list->data[place].val != DEAD_VALUE )
                error |= DataError;
            size++;
            break;
        }
        else if( list->data[place].prev  != FREE_INDEX 
                 ||
                 list->data[place].val != DEAD_VALUE )
        {

            error |= DataError;
            break;
        }
        
        place = list->data[place].next;
        size++;
    }
    
    return ( Status ) error;
}

void ListLinearize( List* list )
{
    list->errors = ListVerify(list);
    if( list->errors != NoErrors)
        return;

    if( list->sorted == 1 && list->data[1].prev != FREE_INDEX )
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
            new_data[i].val = list->data[cur_ptr].val;
        }
        else
        {

            if (list->free == 0) 
                list->free = i;
            
            new_data[i].prev  = FREE_INDEX;
            new_data[i].next  = ( i == list->capacity ? 0 : i + 1 );
            new_data[i].val = DEAD_VALUE;
        }

        cur_ptr = list->data[cur_ptr].next;
    }

    free(list->data);

    list->sorted = 1;
    list->head = 1;
    list->tail = (list->size == 0 ? 1 : list->size);
    list->data = new_data;
}

void print( List* list )
{
    printf("qqqqqq");
    for( int i = 0; i < list->size; i++ )
    {
        printf( "(%d, %d) ", i,list->data[i].val );
    }
    printf( "(%d, %d) ", 9,list->data[9].val );
    printf("\n");
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

    if (index <= list->capacity + 1 && list->data[index].prev == FREE_INDEX)
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
                (list->data[index].prev == FREE_INDEX) ? -1 : list->data[index].prev,
                list->data[index].val, list->data[index].next );
        
        if ( index > 0 ) fprintf( dump_file, "    node%lu -> node%lu;\n", index - 1, index );
    }

    fputs( "\n    edge [style=solid, constraint=false];\n", dump_file );

    for ( size_t index = 1; index <= list->capacity; ++index )
    {
        if ( list->data[index].next != 0 )
            fprintf(dump_file, "    node%lu: <n> -> node%lu;\n", index, list->data[index].next);
        if ( list->data[index].prev != FREE_INDEX && list->data[index].prev != 0 )
            fprintf(dump_file, "    node%lu: <p> -> node%lu;\n", index, list->data[index].prev);
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