#ifndef _RINGBUFFER_API_H_
#define _RINGBUFFER_API_H_

#include <stdint.h>     // for using uint8_t etc types
#include <stddef.h>     // for using size_t type
#include <stdbool.h>    // for using bool type

/**
*   This macro defines how many ring buffers can be in the system
*/
#define RING_BUFFER_MAX 2


/**
*   This is the user ring buffer structure
*   Use this in code to declare ring buffer structure
*/
typedef struct {
    size_t s_elem;
    size_t n_elem;
    void *buffer;
} rb_attr_t;

/**
*   Use this type to describe a 'descriptor' of your ring buffer in the whole system.
*   For example if need only one ring buffer, then only 
*   The max value of this type can be is RING_BUFFER_MAX - 1.
*   
*/
typedef unsigned int rbd_t ;



/* PUBLIC API STARTS HERE */

bool ring_buffer_init(rbd_t * const rbd, const rb_attr_t * const attr);
bool ring_buffer_put(const rbd_t rbd, const void * const data) ;
bool ring_buffer_get(const rbd_t rbd, void *data) ;

size_t ring_buffer_get_capacity( const rbd_t rbd );






#endif
