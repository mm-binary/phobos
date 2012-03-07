/*
 * Placed into the Public Domain.
 * written by Sean Kelly
 * www.digitalmars.com
 */

#ifdef __APPLE__


#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

void _d_gc_addrange( void* pbot, void* ptop );
void _d_gc_removerange( void* p );

struct Array { size_t length; void *ptr; };
extern struct Array _deh_eh_array;

typedef struct
{
    const char* seg;
    const char* sect;
} seg_ref;

const static seg_ref data_segs[] = {{SEG_DATA, SECT_DATA},
                                    {SEG_DATA, SECT_BSS},
                                    {SEG_DATA, SECT_COMMON}};
const static int NUM_DATA_SEGS   = sizeof(data_segs) / sizeof(seg_ref);

#if defined(__LP64__)
static void on_add_image( const struct mach_header_64* h, intptr_t slide )
{
    const struct section_64* sect;
    int i;

    for( i = 0; i < NUM_DATA_SEGS; ++i )
    {
        sect = getsectbynamefromheader_64( h,
                                        data_segs[i].seg,
                                        data_segs[i].sect );
        if( sect == NULL || sect->size == 0 )
            continue;
        _d_gc_addrange( (void*) sect->addr + slide,
                        (void*) sect->addr + slide + sect->size );
    }

    sect = getsectbynamefromheader_64( h,
                                    "__DATA",
                                    "__deh_eh" );
    if (sect && sect->size)
    {
        /* BUG: this will fail if there are multiple images with __deh_eh
         * sections. Not set up to handle that.
         */
        _deh_eh_array.ptr = (void *) sect->addr + slide;
        _deh_eh_array.length = sect->size;
    }
}


static void on_remove_image( const struct mach_header_64* h, intptr_t slide )
{
    const struct section_64* sect;
    int i;

    for( i = 0; i < NUM_DATA_SEGS; ++i )
    {
        sect = getsectbynamefromheader_64( h,
                                        data_segs[i].seg,
                                        data_segs[i].sect );
        if( sect == NULL || sect->size == 0 )
            continue;
        _d_gc_removerange( (void*) sect->addr + slide );
    }
}


void _d_osx_image_init()
{
    _dyld_register_func_for_add_image( &on_add_image );
    _dyld_register_func_for_remove_image( &on_remove_image );
}
#else
static void on_add_image( const struct mach_header* h, intptr_t slide )
{
    const struct section* sect;
        int i;

    for( i = 0; i < NUM_DATA_SEGS; ++i )
    {
        sect = getsectbynamefromheader( h,
                                        data_segs[i].seg,
                                        data_segs[i].sect );
        if( sect == NULL || sect->size == 0 )
            continue;
        _d_gc_addrange( (void*) sect->addr + slide,
                        (void*) sect->addr + slide + sect->size );
    }

    sect = getsectbynamefromheader( h,
                                    "__DATA",
                                    "__deh_eh" );
    if (sect && sect->size)
    {
        /* BUG: this will fail if there are multiple images with __deh_eh
         * sections. Not set up to handle that.
         */
        _deh_eh_array.ptr = (void *) sect->addr + slide;
        _deh_eh_array.length = sect->size;
    }
}


static void on_remove_image( const struct mach_header* h, intptr_t slide )
{
    const struct section* sect;
        int i;

    for( i = 0; i < NUM_DATA_SEGS; ++i )
    {
        sect = getsectbynamefromheader( h,
                                        data_segs[i].seg,
                                        data_segs[i].sect );
        if( sect == NULL || sect->size == 0 )
            continue;
        _d_gc_removerange( (void*) sect->addr + slide );
    }
}


void _d_osx_image_init()
{
    _dyld_register_func_for_add_image( &on_add_image );
    _dyld_register_func_for_remove_image( &on_remove_image );
}
#endif

#endif
