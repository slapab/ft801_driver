#ifndef _FT80X_TASK_H_
#define _FT80X_TASK_H_

#include <stdbool.h>



// function pointers
typedef bool (*painting_task_t)(void * const data);
typedef bool (*doing_task_t)( void * const data );
typedef bool (*gpu_it_task_t)( const uint8_t it_flags, void * const data );

#define FT80X_TASK_FUNC_DOING   (1<<0) ;
#define FT80X_TASK_FUNC_GPU_IT  (1<<1) ;

typedef struct FT80xTask_TypeDef FT80xTask_TypeDef;

struct FT80xTask_TypeDef
{
    /// Task ID
    uint16_t m_id ;
    
    /// Which functions task handle
    uint16_t m_func_mask ;
    
    /// The shared memory space to use between task's funciton
    void * mp_shared_data ;
    
    /// Task functions:
    painting_task_t mfp_painting ;
    doing_task_t    mfp_doing ;
    gpu_it_task_t   mfp_gpu_it ;
    
    
    // do not init this
    FT80xTask_TypeDef * next ;
    
} ;



#endif

