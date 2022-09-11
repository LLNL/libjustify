#include <stdbool.h>    // true and false
#include <stddef.h>     // NULL
#include <assert.h>     // assert
#include <string.h>     // strncmp
#include <stdlib.h>     // calloc
#include <stdio.h>      // fprintf
#include <string.h>     // strspan
#include <stdarg.h>     // variadic
#include <wchar.h>      // wint_t
#include <uchar.h>
#include <stdint.h>     // intmax_t
#include "cprintf.h"

static struct atom * origin = NULL;

void
dump_atom( struct atom * a ){
    fprintf(stdout, "%s:%d orig=%6s w=%4zu wmax=%4zu a=%14p a->up=%14p a->down=%14p a->left=%14p a->right=%14p\n",
            __FILE__, __LINE__, 
            a->original_specification, a->original_field_width, a->new_field_width,
            a, a->up, a->down, a->left, a->right );
}

void
dump_graph( void ){
    struct atom *a = origin, *c;
    while( NULL != a ){
        // Address of this atom.
        c = a;
        while( NULL != c ){
            printf("p=%-20p", c );
            c = c->right;
        }
        printf("\n");

        // Address of atom to the left.
        c = a;
        while( NULL != c ){
            printf("l=%-20p", c->left );
            c = c->right;
        }
        printf("\n");

        // Address of atom to the right.
        c = a;
        while( NULL != c ){
            printf("r=%-20p", c->right );
            c = c->right;
        }
        printf("\n");

        // Address of atom above.
        c = a;
        while( NULL != c ){
            printf("u=%-20p", c->up );
            c = c->right;
        }
        printf("\n");

        // Address of atom below.
        c = a;
        while( NULL != c ){
            printf("d=%-20p", c->down );
            c = c->right;
        }
        printf("\n");

        // is this atom a conversion specification?
        c = a;
        while( NULL != c ){
            printf("isconvspec=%-11c", c->is_conversion_specification ? 't' : 'f' );
            c = c->right;
        }
        printf("\n");

        // pointer to the ordinary text
        c = a;
        while( NULL != c ){
            printf("o=%-20p", c->ordinary_text );
            c = c->right;
        }
        printf("\n");

        // pointer to original specification
        c = a;
        while( NULL != c ){
            printf("orig=%-17s", c->original_specification );
            c = c->right;
        }
        printf("\n");

        // pointer to new specification
        c = a;
        while( NULL != c ){
            printf("new= %-17s", c->new_specification );
            c = c->right;
        }
        printf("\n");


        printf("\n");
        a = a->down;
    }
    fflush(NULL);
}

void
free_graph( struct atom *a ){
    if( NULL == origin ){
        return;
    }
    if( NULL == a ){
        a = origin;
    }
    if( a->right ){
        free_graph( a->right );
    }
    if( a->down ){
        free_graph( a->down );
    }
    if( a == origin ){
        origin = NULL;
    }
    if( a->up ){
        a->up->down = NULL;
    }
    if( a->left ){
        a->left->right = NULL;
    }
    free( a );
    return;
}

struct atom *
create_atom( bool is_newline ){
    static struct atom *last_atom_on_last_line = NULL;
    static struct atom *first_atom_on_last_line = NULL;

    struct atom *a = calloc( sizeof( struct atom ), 1 );
    assert(a);
    
    // recall the value of NULL is implementation-specific.
    a->original_specification       = NULL;
    a->new_specification            = NULL;

    a->flags                        = NULL;
    a->field_width                  = NULL;
    a->precision                    = NULL;
    a->length_modifier              = NULL;
    a->conversion_specifier         = NULL;

    a->ordinary_text                = NULL;
    a->pargs                        = NULL;

    a->right                        = NULL;
    a->left                         = NULL;
    a->up                           = NULL;
    a->down                         = NULL;

    if( NULL == origin ){
        first_atom_on_last_line = origin = a;
    }else if(is_newline){
        // new line, only need to set up and down.
        first_atom_on_last_line->down = a;
        a->up = first_atom_on_last_line;
        first_atom_on_last_line = a;
    }else{
        // adding to an existing line, need to set
        // left/right and (maybe) up/down.
        last_atom_on_last_line->right = a;
        a->left = last_atom_on_last_line;
        if(last_atom_on_last_line->up){
            a->up = last_atom_on_last_line->up->right;
            last_atom_on_last_line->up->right->down = a;
        }
    }
    last_atom_on_last_line = a;
    return a;
}


// Conversion specifications look like this:
// %[flags][field_width][.precision][length_modifier]specifier


ptrdiff_t
parse_flags( const char *p ){
    // Returns the number of bytes starting from the beginning
    // of p that consist only of the characters #0- +'I
    return strspn( p, "#0- +'I" );
}

ptrdiff_t
parse_field_width( const char *p ){
    // Returns the number of bytes in the field width,
    // or zero of no field width specified.
    char *end;
    assert( '*' != *p );    // Don't support * or *n$ (yet).
    strtol( p, &end, 10 );
    return end - p;
}

 ptrdiff_t
parse_precision( const char *p ){
    char *end;
    // Returns the number of bytes representing
    // precision, including the leading '.'.  
    if( '.' == *p ){
        assert( '*' != *(p+1) ); // Don't support * or *n$ (yet).
        strtol( p+1, &end, 10 );
        return (end - p); 
    }else{
        return 0;
    }
}

ptrdiff_t 
parse_length_modifier( const char *p ){
    // length modifiers are:
    // h, hh, l, ll, L, q, j, z, t
    return strspn( p, "hlLqjzt" );
}

ptrdiff_t 
parse_conversion_specifier( const char *p ){
    // conversion specifiers are:
    // d, i, o, u, x, X, e, E, f, F, g, G, a, A, c, C, s, S, p, n, m, %
    size_t d = strspn( p, "diouxXeEfFgGaAcCsSpnm%" );
    // This one is mandatory and there can only be one.
    assert( d==1 );
    return d;
}

void
archive( const char *p, ptrdiff_t span, char **q ){
    static char *empty = "";
    if( 0 == span ){
        *q = empty;
    }else{
        *q = calloc( span+1, 1 );
        strncpy( *q, p, span );
    }
}

bool
is( char *p, const char *q ){
    // return true if the strings are identical.
    size_t len = strlen( q );
    return (bool) ! strncmp( p, q, len );
}
static void
calc_actual_width( struct atom *a ){
    // FIXME Not (yet) supporting 'n' as a conversion specifier.
    // Reproduces the big table at 
    // https://en.cppreference.com/w/c/io/fprintf
/*
    length      conversion
    modifier    specifier       type
    --------------------------------
    (none)      c               int
    l           c               wint_t
    (none)      s               char*
    l           s               wchar_t*
    hh          d/i             signed char [int]
    h           d/i             short [int]
    (none)      d/i             int
    l           d/i             long
    ll          d/i             long long
    j           d/i             intmax_t
    z           d/i             signed size_t [ssize_t]
    t           d/i             ptrdiff_t
    hh          o/x/X/u         unsigned char [int]
    h           o/x/X/u         unsigned short [int]
    (none)      o/x/X/u         unsigned int
    l           o/x/X/u         unsigned long
    ll          o/x/X/u         unsigned long long
    j           o/x/X/u         uintmax_t
    z           o/x/X/u         size_t
    t           o/x/X/u         unsigned ptrdiff_t [???]
    (none)/l    f/F/e/E/a/A/g/G double
    L           f/F/e/E/a/A/g/G long double
    (none)      p               void*
*/
    static char buf[4097]; 
    if( is(a->conversion_specifier, "c") ){
        if( is( a->length_modifier, "" ) ){
            a->type = C_INT;
            a->val.c_int = va_arg( *(a->pargs), int );
            snprintf( buf, 4096, a->original_specification, a->val.c_int );
        }else if( is( a->length_modifier, "l" ) ){
            a->type = C_WINT_T;
            a->val.c_wint_t = va_arg( *(a->pargs), wint_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_wint_t );
        }else{
            assert(0);
        }
    }else if( is(a->conversion_specifier, "s") ){
        if( is( a->length_modifier, "" ) ){
            a->type = C_CHARX;
            a->val.c_charx = va_arg( *(a->pargs), char* );
            snprintf( buf, 4096, a->original_specification, a->val.c_charx );
        }else if( is( a->length_modifier, "l" ) ){
            a->type = C_WCHAR_TX;
            a->val.c_wchar_tx = va_arg( *(a->pargs), wchar_t* );
            snprintf( buf, 4096, a->original_specification, a->val.c_wchar_tx );
        }else{
            assert(0);
        }
    }else if( is(a->conversion_specifier, "d") 
          ||  is(a->conversion_specifier, "i") ){
        if( is( a->length_modifier, "hh" ) 
        ||  is( a->length_modifier, "h" ) 
        ||  is( a->length_modifier, "" ) ){
            a->type = C_INT;
            a->val.c_int = va_arg( *(a->pargs), int );
            snprintf( buf, 4096, a->original_specification, a->val.c_int );
        }else if( is( a->length_modifier, "l" ) ){
            a->type = C_LONG;
            a->val.c_long = va_arg( *(a->pargs), long );
            snprintf( buf, 4096, a->original_specification, a->val.c_long );
        }else if( is( a->length_modifier, "ll" ) ){
            a->type = C_LONG_LONG;
            a->val.c_long_long = va_arg( *(a->pargs), long long );
            snprintf( buf, 4096, a->original_specification, a->val.c_long_long );
        }else if( is( a->length_modifier, "j" ) ){
            a->type = C_INTMAX_T;
            a->val.c_intmax_t = va_arg( *(a->pargs), intmax_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_intmax_t );
        }else if( is( a->length_modifier, "z" ) ){
            a->type = C_SSIZE_T;
            a->val.c_ssize_t = va_arg( *(a->pargs), ssize_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_ssize_t );
        }else if( is( a->length_modifier, "t" ) ){
            a->type = C_PTRDIFF_T;
            a->val.c_ptrdiff_t = va_arg( *(a->pargs), ptrdiff_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_ptrdiff_t );
        }else{
            assert(0);
        }
    }else if( is(a->conversion_specifier, "o") 
          ||  is(a->conversion_specifier, "x") 
          ||  is(a->conversion_specifier, "X") 
          ||  is(a->conversion_specifier, "u") ){
        if( is( a->length_modifier, "hh" ) 
        ||  is( a->length_modifier, "h" ) ){
            a->type = C_INT;
            a->val.c_int = va_arg( *(a->pargs), int );
            snprintf( buf, 4096, a->original_specification, a->val.c_int );
        }else if( is( a->length_modifier, "" ) ){
            a->type = C_UNSIGNED_INT;
            a->val.c_unsigned_int = va_arg( *(a->pargs), unsigned int );
            snprintf( buf, 4096, a->original_specification, a->val.c_unsigned_int );
        }else if( is( a->length_modifier, "l" ) ){
            a->type = C_UNSIGNED_LONG;
            a->val.c_unsigned_long = va_arg( *(a->pargs), unsigned long );
            snprintf( buf, 4096, a->original_specification, a->val.c_unsigned_long );
        }else if( is( a->length_modifier, "ll" ) ){
            a->type = C_UNSIGNED_LONG_LONG;
            a->val.c_unsigned_long_long = va_arg( *(a->pargs), unsigned long long );
            snprintf( buf, 4096, a->original_specification, a->val.c_unsigned_long_long );
        }else if( is( a->length_modifier, "j" ) ){
            a->type = C_UINTMAX_T;
            a->val.c_uintmax_t = va_arg( *(a->pargs), uintmax_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_uintmax_t );
        }else if( is( a->length_modifier, "z" ) ){
            a->type = C_SIZE_T;
            a->val.c_size_t = va_arg( *(a->pargs), size_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_size_t );
        }else if( is( a->length_modifier, "t" ) ){
            a->type = C_PTRDIFF_T;
            a->val.c_ptrdiff_t = va_arg( *(a->pargs), ptrdiff_t );
            snprintf( buf, 4096, a->original_specification, a->val.c_ptrdiff_t );
        }else{
            assert(0);
        }
    }else if( is(a->conversion_specifier, "f") 
          ||  is(a->conversion_specifier, "F") 
          ||  is(a->conversion_specifier, "e") 
          ||  is(a->conversion_specifier, "E") 
          ||  is(a->conversion_specifier, "a") 
          ||  is(a->conversion_specifier, "A") 
          ||  is(a->conversion_specifier, "g") 
          ||  is(a->conversion_specifier, "G") ){
        if( is( a->length_modifier, "l" )
        ||  is( a->length_modifier, "" ) ){
            a->type = C_DOUBLE;
            a->val.c_double = va_arg( *(a->pargs), double );
            snprintf( buf, 4096, a->original_specification, a->val.c_double );
        }else if( is( a->length_modifier, "L" ) ){
            a->type = C_LONG_DOUBLE;
            a->val.c_long_double = va_arg( *(a->pargs), long double );
            snprintf( buf, 4096, a->original_specification, a->val.c_long_double );
        }else{
            assert(0);
        }
    }else if( is(a->conversion_specifier, "p") ){
        if( is( a->length_modifier, "" ) ){
            a->type = C_VOIDX;
            a->val.c_voidx = va_arg( *(a->pargs), void* );
            snprintf( buf, 4096, a->original_specification, a->val.c_voidx );
        }else{
            assert(0);
        }
    }else{
        assert(0);
    }
    assert( strnlen(buf, 4096) < 4095 );
    a->original_field_width = strlen( buf );
}

void
calc_max_width(){
    struct atom *a = origin, *c;
    assert( NULL != a );
    size_t w = 0;
    while( NULL != a ){
        if( a->is_conversion_specification ){
            c = a;
            while( NULL != c ){
                // find max field width
                if( c->original_field_width > w ){
                    w = c->original_field_width;
                }
                c = c->down;
            }
            c = a;
            while( NULL != c){
                // set max field width
                c->new_field_width = w;
                c = c->down;
            }
            w = 0;
        }
        a = a->right;
    }
}

void
generate_new_specs(){
    char buf[4099];
    int rc;
    struct atom *a = origin, *c;
    assert( NULL != a );
    while( NULL != a ){
        if( a->is_conversion_specification ){
            c = a;
            while( NULL != c ){
                rc = snprintf(buf, 4099, "%%%s%zu%s%s%s",
                        c->flags,
                        c->new_field_width,
                        c->precision,
                        c->length_modifier,
                        c->conversion_specifier);
                assert( rc < 4099 );
                archive( buf, strlen(buf), &(c->new_specification));
                c = c->down;
            }
        }
        a = a->right;
    }
}

void
print_something_already(){
    struct atom *a = origin, *c;
    assert( NULL != a );
    while( NULL != a ){
        c = a;
        while( NULL != c ){
            if( c->is_conversion_specification ){
                switch( c->type ){
                    case C_INT:                 printf( c->new_specification, c->val.c_int );                break;
                    case C_WINT_T:              printf( c->new_specification, c->val.c_wint_t );             break;
                    case C_CHARX:               printf( c->new_specification, c->val.c_charx );              break;
                    case C_WCHAR_TX:            printf( c->new_specification, c->val.c_wchar_tx );           break;
                    case C_LONG:                printf( c->new_specification, c->val.c_long );               break;
                    case C_LONG_LONG:           printf( c->new_specification, c->val.c_long_long );          break;
                    case C_INTMAX_T:            printf( c->new_specification, c->val.c_intmax_t );           break;
                    case C_SSIZE_T:             printf( c->new_specification, c->val.c_ssize_t );            break;
                    case C_PTRDIFF_T:           printf( c->new_specification, c->val.c_ptrdiff_t );          break;
                    case C_UNSIGNED_INT:        printf( c->new_specification, c->val.c_unsigned_int );       break;
                    case C_UNSIGNED_LONG:       printf( c->new_specification, c->val.c_unsigned_long );      break;
                    case C_UNSIGNED_LONG_LONG:  printf( c->new_specification, c->val.c_unsigned_long_long ); break;
                    case C_UINTMAX_T:           printf( c->new_specification, c->val.c_uintmax_t );          break;
                    case C_SIZE_T:              printf( c->new_specification, c->val.c_size_t );             break;
                    case C_DOUBLE:              printf( c->new_specification, c->val.c_double );             break;
                    case C_LONG_DOUBLE:         printf( c->new_specification, c->val.c_long_double );        break;
                    case C_VOIDX:               printf( c->new_specification, c->val.c_voidx );              break;
                    default:
                                                assert(0);
                                                break;
                }
            }else{
                printf( "%s", c->ordinary_text );
            }
            c = c->right;
        }
        a = a->down;
    }
}

void
cprintf( const char *fmt, ... ){
    va_list args;
    struct atom *a;
    const char *p = fmt, *q = fmt;
    ptrdiff_t d = 0;
    ptrdiff_t span;
    /* There's a reasonable argument that newlines should be indicated by
       '\n' in the ordinary text, which would allow successive calls to 
       cprintf() to populate a single line.  This raises, however, the
       question of what to do with cprintf("\n\n") and similar.  For now,
       keep parsing easy.
    */
    bool is_newline = true;
    va_start( args, fmt );
    while( *p != '\0' ){
        d = strcspn( p, "%" ); 
        q = p;
        if( d == 0 ){
            // We've found a converstion specification.
            a = create_atom( is_newline );
            a->pargs = &args;
            a->is_conversion_specification = true;

            q++; // Skip over initial '%'

            span = parse_flags( q );
            archive( q, span, &(a->flags) );
            q += span;
            
            span = parse_field_width( q );
            archive( q, span, &(a->field_width) );
            q += span;

            span = parse_precision( q );
            archive( q, span, &(a->precision) );
            q += span;

            span = parse_length_modifier( q );
            archive( q, span, &(a->length_modifier) );
            q += span;

            span = parse_conversion_specifier( q );
            archive( q, span, &(a->conversion_specifier) );
            q += span;

            archive( p, q-p, &(a->original_specification) ); 

            calc_actual_width( a );
            a->pargs = NULL;    // cleanup
            p = q;
        }else{
            // We've found some normal text.
            a = create_atom( is_newline );
            a->is_conversion_specification = false;
            archive( q, d, &(a->ordinary_text) );
            q += d;
            p = q;
        }
        is_newline = false;
    }
    va_end(args);
}

void
cflush(){ 
    calc_max_width();
    generate_new_specs();
    print_something_already();
}
