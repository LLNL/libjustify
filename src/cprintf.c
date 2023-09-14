// Copyright 2023 Lawrence Livermore National Security, LLC and other
// libjustify Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdbool.h>    // true and false
#include <stddef.h>     // NULL
#include <assert.h>     // assert
#include <string.h>     // strncmp
#include <stdlib.h>     // calloc
#include <stdio.h>      // fprintf
#include <string.h>     // strspan
#include <stdarg.h>     // variadic
#include <wchar.h>      // wint_t
#include <stdint.h>     // intmax_t
#include <uchar.h>
#include <cprintf.h>


// These are the types that printf and friends are aware of.
// Recall that types shorter than int (e.g., char and short)
// are promoted to int when accessed by va_arg().
typedef union{
    int                 c_int;
    wint_t              c_wint_t;
    char*               c_charx;
    wchar_t*            c_wchar_tx;
    long                c_long;
    long long           c_long_long;
    intmax_t            c_intmax_t;
    ssize_t             c_ssize_t;
    ptrdiff_t           c_ptrdiff_t;
    unsigned int        c_unsigned_int;
    unsigned long       c_unsigned_long;
    unsigned long long  c_unsigned_long_long;
    uintmax_t           c_uintmax_t;
    size_t              c_size_t;
    double              c_double;
    long double         c_long_double;
    void*               c_voidx;
    int*                c_intp;
}value;

typedef enum{
    C_INT,
    C_WINT_T,
    C_CHARX,
    C_WCHAR_TX,
    C_LONG,
    C_LONG_LONG,
    C_INTMAX_T,
    C_SSIZE_T,
    C_PTRDIFF_T,
    C_UNSIGNED_INT,
    C_UNSIGNED_LONG,
    C_UNSIGNED_LONG_LONG,
    C_UINTMAX_T,
    C_SIZE_T,
    C_DOUBLE,
    C_LONG_DOUBLE,
    C_VOIDX,
    C_INT_PTR
}type_t;

struct atom{
    // Atoms should be allocated with calloc().  However, the value of
    // NULL is implementation-dependent, so be sure any new pointers
    // added here are explicitly set to NULL in create_atom() and freed
    // in free_graph();
    bool is_conversion_specification;
    size_t original_field_width;
    size_t new_field_width;

    char *original_specification;
    char *new_specification;

    char *flags;
    char *field_width;
    char *precision;
    char *length_modifier;
    char *conversion_specifier;

    char *ordinary_text;
    bool is_dummy;

    va_list *pargs;
    type_t type;
    value  val;

    // navigation
    struct atom *right;
    struct atom *left;
    struct atom *up;
    struct atom *down;
};


// Dummy rows represents the last top and bottom rows of the graph.
struct dummy_rows_ds {
    struct atom *upper;
    struct atom *lower;
    struct atom *bot_root; // Stores the root of the bottom row. This is used for insertion.
    struct atom *top_root; // Allows free_graph() to be a bit dumber. TODO: THIS IS ONLY USED FOR FREE_GRAPH().  REMOVE IT.
};

void dump_graph( void );
void _free_graph( struct atom *a );
void free_graph();

struct atom * create_atom( bool is_newline );
ptrdiff_t parse_flags( const char *p );
ptrdiff_t parse_field_width( const char *p );
ptrdiff_t parse_precision( const char *p );
ptrdiff_t parse_length_modifier( const char *p );
ptrdiff_t parse_conversion_specifier( const char *p );
void archive( const char *p, ptrdiff_t span, char **q );
bool is( char *p, const char *q );
void _cprintf( FILE *stream, const char *fmt, va_list *args );
void exit_nice(void);
void calculate_writeback(struct atom * a);


struct atom * _make_dummy( void);
void _extend_dummy_rows(size_t size);
void _reconnect_rows(void);

static struct atom *origin = NULL;
static struct dummy_rows_ds *dummy_rows = NULL;
static FILE *dest = NULL;


struct atom *
_make_dummy( void ) {
    struct atom *a = calloc( sizeof( struct atom ), 1 );
    assert(a);

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

    a->is_dummy                     = true;

    a->is_conversion_specification  = false;
    return a;
};


void
_extend_dummy_rows(size_t size) {
    struct atom * new_top;
    struct atom * new_bottom;

    for (; size != 0; --size){
        new_top = _make_dummy();
        new_bottom = _make_dummy();

        new_top->down = new_bottom;
        new_bottom->up = new_top;

        if (NULL == dummy_rows){
            dummy_rows = calloc(1, sizeof(struct dummy_rows_ds));
            if (NULL == dummy_rows) {
                fprintf(stderr, "Memory allocation failed.\n");
                exit(EXIT_FAILURE);
            }
            dummy_rows->bot_root = new_bottom;
            dummy_rows->top_root = new_top;
        } else {
            new_top->left = dummy_rows->upper;
            new_bottom->left = dummy_rows->lower;
            dummy_rows->upper->right = new_top;
            dummy_rows->lower->right = new_bottom;
        }

        assert(dummy_rows != NULL);
        dummy_rows->upper = new_top;
        dummy_rows->lower = new_bottom;
    }
};

void
dump_graph( void ){
    struct atom *a = origin->up, *c;
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
            printf("new =%-17s", c->new_specification );
            c = c->right;
        }
        printf("\n");

        c = a;
        while( NULL != c ){
            printf("dummy =%-17d", c->is_dummy );
            c = c->right;
        }
        printf("\n");


        printf("\n");
        a = a->down;
    }
    fflush(NULL);
}

//TODO CLEAN THIS UP. USING DUMMY_ROWS->UPPER AS THE ROOT OF THE UPPER IS VERY HACKY.
void
_free_graph( struct atom *a ){
    // Check to see if any graph exists.
    if( NULL == dummy_rows->top_root){
        return;
    }

    // Starts the process.
    if( NULL == a ){
        a = dummy_rows->top_root;
    }

    // Find the rightmost atom in the top line.
    if( a->right ){
        _free_graph( a->right );
    }

    // For each atom in the top line, find the last atom in that column.
    if( a->down ){
        _free_graph( a->down );
    }

    // If we're at the point where we're looking at the atom in the top left
    // corner, we're done.
    if (a == origin){
        origin = NULL;
    }

    // If there's an atom above us, disconnect from it.
    if( a->up ){
        a->up->down = NULL;
    }

    // If there's an atom to the left of us, disconnect from it.
    if( a->left ){
        a->left->right = NULL;
    }

    free( a->original_specification );
    free( a->new_specification );
    free( a->flags );
    free( a->field_width );
    free( a->precision );
    free( a->length_modifier );
    free( a->conversion_specifier );
    free( a->ordinary_text );
    free( a );

    return;
}

void
free_graph(){
    _free_graph( origin->up ); //Go to the dummy row. This is kinda convoluted
    free( dummy_rows );
    dummy_rows = NULL;
}


struct atom *
create_atom( bool is_newline ){
    const size_t extend_by = 1;
    static struct atom *last_atom_on_last_line = NULL;

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
    

    //Origin 
    if( NULL == origin ){
        _extend_dummy_rows( extend_by );
        a->down = dummy_rows->bot_root;
        origin = a;
    }else if(is_newline){
        a->down = dummy_rows->bot_root;
    } else {
        //NOTE: 1) It's probably better to build the first row of true atoms and
        //         Then create the dummy rows. Speed up will be proprotional to 
        //         the number of Atoms in the first row.
        //      2) It also MIGHT be more efficient during extension to create a 
        //         couple of anticipated dummies. 
        //
        //TODO:    Profile the latter change and modify for the former.
        //TODO:    This isn't gonna be  memory contrained, I can for sure 
        //         spare a few pointers to make this less horrible. 
        if ( NULL == last_atom_on_last_line->down->right ){
            _extend_dummy_rows( extend_by );
            last_atom_on_last_line->down->right = dummy_rows->lower;
        }

        a->down = last_atom_on_last_line->down->right;
        last_atom_on_last_line->right = a;
        a->left = last_atom_on_last_line;
    }
    
    a->up = a->down->up;
    a->up->down = a;
    a->down->up = a;
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
    // This will call calloc for null strings (strings with
    // a length of 0 consisting only of a terminating null).
    // This allows _free_graph() to be a little dumber.
    *q = calloc( span+1, 1 );
    strncpy( *q, p, span );
}

bool
is( char *p, const char *q ){
    // return true if the strings are identical.  Note either p or q
    // may be an empty string on length 0.
    size_t lenp = strlen( p );
    size_t lenq = strlen( q );
    return lenp == lenq ? (bool) ! strncmp( p, q, lenq ) : false;
}
static void
calc_actual_width( struct atom *a ){
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
    t           o/x/X/u         unsigned ptrdiff_t [?]
    (none)/l    f/F/e/E/a/A/g/G double
    L           f/F/e/E/a/A/g/G long double
    (none)      p               void*
    (none)      n               int*
*/
    if(a->is_dummy) return; //Return early if this is a dummy atom. TODO: This isn't great fix it.

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
    }else if ( is(a->conversion_specifier, "n") ){ // This is a writeback
        if( is( a->length_modifier, "") ){
            a->type = C_INT_PTR;
            a->val.c_intp = va_arg( *(a->pargs), int* );
            a->original_field_width = 0;
        } else {
            assert(0);
        }
        return; //TODO check if this is better trying to set field width to buf
    
    }else{
        assert(0);
    }
    assert( strnlen(buf, 4096) < 4095 );
    a->original_field_width = strlen( buf );
}

void
calc_max_width(){
    // Really can't remember why I put this here but it can't hurt
    assert( dummy_rows != NULL ); 
    struct atom *aiter = origin, *citer, *diter; //A is the top dummy
    assert( NULL != aiter || NULL != aiter->up);
    diter = dummy_rows->top_root;

    size_t w = 0;
    while ( NULL != diter){
        aiter = diter->down;
        if( aiter->is_conversion_specification ){
            citer = aiter;
            while( citer->is_dummy == false && NULL != citer->down){ //last is a sanity check
                // find max field width
                
                if( citer->original_field_width > w ){
                    w = citer->original_field_width;
                }
                citer = citer->down;
            }
            citer = aiter;
            while( citer->is_dummy == false){ //makes clean up easier
                // set max field width
                citer->new_field_width = w;
                citer = citer->down;
            }
            w = 0;
        }
        diter = diter->right;
    }
}

void generate_new_specs(){
    char buf[4099];
    int rc;
    struct atom *a = dummy_rows->top_root, *c; //A is the top dummy row.
    assert( NULL != a );
    while( NULL != a){
        c = a;
        while( NULL != c ){
            if( c->is_conversion_specification ){ 
                rc = snprintf(buf, 4099, "%%%s%zu%s%s%s",
                        c->flags,
                        c->new_field_width,
                        c->precision,
                        c->length_modifier,
                        c->conversion_specifier);
                assert( rc < 4099 );
                archive( buf, strlen(buf), &(c->new_specification));
            }
            c = c->down;
        }
        a = a->right;
    }
}

/*void
generate_new_specs(){
    char buf[4099];
    int rc;
    struct atom *a = dummy_rows->top_root, *c; //A is the top dummy row.
    assert( NULL != a );
    while( NULL != a){
        if( a->down->is_conversion_specification ){ 
            c = a->down;
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
}*/

void 
calculate_writeback(struct atom * a) {
    // Calculate writeback handles %n specifiers traversing right to left summing up the field widths
    // and then writing back the total to the pointer
    int sum = 0;
    for(struct atom *c = a; NULL != c; c = c->left) {
        if(c->is_conversion_specification) {
            sum += c->new_field_width;
        } else if (c->ordinary_text) {
            sum += strlen(c->ordinary_text);
        } else {
            assert(0);
        }
    }
    if (a->val.c_intp != NULL) {
        *a->val.c_intp = sum; //Writeback 
    } else {
        // handle error
        fprintf(stderr, "Error: a->val.c_intp is NULL\n");
    }
}

void
print_something_already(){
    // bunch of checks to see if Something horrible happened... No dummies. 
    // TODO: REMOVE IN PROD
    //assert( NULL != origin && NULL != origin->up);
    struct atom *a = origin, *c;
    assert( NULL != a);
    assert( NULL != a->down); 

    while ( NULL != a && a != dummy_rows->bot_root) {
        c = a;
        while ( NULL != c) {
            if( c->is_conversion_specification ){
                switch( c->type ){
                    case C_INT_PTR:             calculate_writeback(c);                                             break;
                    case C_INT:                 fprintf( dest, c->new_specification, c->val.c_int );                break;
                    case C_WINT_T:              fprintf( dest, c->new_specification, c->val.c_wint_t );             break;
                    case C_CHARX:               fprintf( dest, c->new_specification, c->val.c_charx );              break;
                    case C_WCHAR_TX:            fprintf( dest, c->new_specification, c->val.c_wchar_tx );           break;
                    case C_LONG:                fprintf( dest, c->new_specification, c->val.c_long );               break;
                    case C_LONG_LONG:           fprintf( dest, c->new_specification, c->val.c_long_long );          break;
                    case C_INTMAX_T:            fprintf( dest, c->new_specification, c->val.c_intmax_t );           break;
                    case C_SSIZE_T:             fprintf( dest, c->new_specification, c->val.c_ssize_t );            break;
                    case C_PTRDIFF_T:           fprintf( dest, c->new_specification, c->val.c_ptrdiff_t );          break;
                    case C_UNSIGNED_INT:        fprintf( dest, c->new_specification, c->val.c_unsigned_int );       break;
                    case C_UNSIGNED_LONG:       fprintf( dest, c->new_specification, c->val.c_unsigned_long );      break;
                    case C_UNSIGNED_LONG_LONG:  fprintf( dest, c->new_specification, c->val.c_unsigned_long_long ); break;
                    case C_UINTMAX_T:           fprintf( dest, c->new_specification, c->val.c_uintmax_t );          break;
                    case C_SIZE_T:              fprintf( dest, c->new_specification, c->val.c_size_t );             break;
                    case C_DOUBLE:              fprintf( dest, c->new_specification, c->val.c_double );             break;
                    case C_LONG_DOUBLE:         fprintf( dest, c->new_specification, c->val.c_long_double );        break;
                    case C_VOIDX:               fprintf( dest, c->new_specification, c->val.c_voidx );              break;
                    default:
                                                assert(0);
                                                break;
                }
            } else if ( c->is_dummy == false ){
                printf( "%s", c->ordinary_text );
            }
            c = c->right;
        }
        a = a->down;
    }
}

void
_cprintf( FILE *stream, const char *fmt, va_list *args ){
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

    if( dest == NULL ){
        dest = stream;
    }
    // This fails if subsequent streams don't match the initial one.
    assert( dest == stream );

    atexit( exit_nice ); // Set exit Callback

    while( *p != '\0' ){
        d = strcspn( p, "%" ); 
        q = p;
        if( d == 0 ){
            // We've found a converstion specification.
            a = create_atom( is_newline );
            a->pargs = args;
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
}

//Callback for exit() to free memory
void exit_nice(void){
    if( NULL != dummy_rows) {
        free_graph();
    }
    exit(0);
}

void
cprintf( const char *fmt, ... ){
    va_list args;
    va_start( args, fmt );
    _cprintf( stdout, fmt, &args );
    va_end(args);
}

void
cfprintf( FILE *stream, const char *fmt, ... ){
    va_list args;
    va_start( args, fmt );
    _cprintf( stream, fmt, &args );
    va_end(args);
}

void
cvprintf( const char *fmt, va_list args ){
    va_list args2;
    va_copy( args2, args );
    _cprintf( stdout, fmt, &args2 );
    va_end(args2);
}

void
cvfprintf( FILE *stream, const char *fmt, va_list args ){
    va_list args2;
    va_copy( args2, args );
    _cprintf( stream, fmt, &args2 );
    va_end(args2);
}

void
cflush(){ 
    if( NULL != origin){ //Just for safety
        calc_max_width();
        generate_new_specs();
        print_something_already();
        free_graph();
    }
    dest = NULL;
}
