
#include <string.h>         // memcpy()
#include "ringBuffer_api.h"


/* Do not use this outside of this file */
struct ring_buffer
{
    size_t s_elem;
    size_t n_elem;
    uint8_t *buf;
    volatile size_t head;
    volatile size_t tail;
};





/*
* This local table is a storage to contain all ring buffers in the system
*/
static struct ring_buffer _rb[RING_BUFFER_MAX];





//  ******************** LOCAL / HELPER FUNCTIONS  ****************************

static bool _ring_buffer_full(struct ring_buffer *rb)
{
    return ((rb->head - rb->tail) == rb->n_elem) ? true : false ;
}
 
static bool _ring_buffer_empty(struct ring_buffer *rb)
{
    return ((rb->head - rb->tail) == 0U) ? true : false;
}








//  ******************** PUBLIC API STARTS HERE ****************************



bool ring_buffer_init( rbd_t * const rbd, const rb_attr_t * const attr)
{
    static int idx = 0;
    bool err = false; 
 
    if ((idx < RING_BUFFER_MAX) && (rbd != NULL) && (attr != NULL)) {
        if ((attr->buffer != NULL) && (attr->s_elem > 0)) {
            /* Check that the size of the ring buffer is a power of 2 */
            if (((attr->n_elem - 1) & attr->n_elem) == 0) {
                /* Initialize the ring buffer internal variables */
                _rb[idx].head = 0;
                _rb[idx].tail = 0;
                _rb[idx].buf = attr->buffer;
                _rb[idx].s_elem = attr->s_elem;
                _rb[idx].n_elem = attr->n_elem;
 
                (*rbd) = idx++;
                err = true ;
            }
        }
    }
 
    return err;
}






bool ring_buffer_put(const rbd_t rbd, const void * const data)
{
    bool err = true ;
 
    if ((rbd < RING_BUFFER_MAX) && (_ring_buffer_full(&_rb[rbd]) == false)) {
        const size_t offset = (_rb[rbd].head & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(&(_rb[rbd].buf[offset]), data, _rb[rbd].s_elem);
        _rb[rbd].head++;
    } else {
        err = false ;
    }
 
    return err;
}




bool ring_buffer_get(const rbd_t rbd, void *data)
{
    bool err = true;
 
    if ((rbd < RING_BUFFER_MAX) && (_ring_buffer_empty(&_rb[rbd]) == false)) {
        const size_t offset = (_rb[rbd].tail & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(data, &(_rb[rbd].buf[offset]), _rb[rbd].s_elem);
        _rb[rbd].tail++;
    } else {
        err = false ;
    }
 
    return err;
}



size_t ring_buffer_capacity( const rbd_t rbd )
{
    return (size_t)(_rb[rbd].head - _rb[rbd].tail );
}


size_t ring_buffer_get_size( const rbd_t rbd )
{
    uint8_t * pBuff = (uint8_t*)_rb[rbd].buf ;
    uint8_t * pElement = (uint8_t*)&_rb[rbd].buf[0] ;
    
    return sizeof(pBuff)/(sizeof(pElement)*_rb[rbd].s_elem) ;
}
