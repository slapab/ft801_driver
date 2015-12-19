
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ft80x_engine_it.h"
#include "ft80x_task.h"



// Display Task states
typedef enum
{
    New = 0,    // this state tells that task was just switched
    Painting,   // task is sending now commands to de GPU
    Running     // task has sent cmds to the GPU, now it can do actual job
} TaskState_TypeDef ;




// Local engine structure
typedef struct
{
    // head for list for storing the registered tasks
    FT80xTask_TypeDef * m_taskListHead ;
    
    // current task ptr
    FT80xTask_TypeDef * m_currTask ;
    
    // current task state
    TaskState_TypeDef m_currTaskState ;
    
    // GPU interrupt flag
    sig_atomic_t m_gpuIT_flag ;
    
    // GPU FLAGS register value
    uint8_t m_gpu_itflags ;
    
    
    
    
    
} ft80x_engine_it_TypeDef ;

/// IF it from GPU has arisen, 0 when handled this information
#define GPU_IT_NEED_READ (1 << 0)
/// FLAGS from GPU has been read but not handled yet, 0 if was handled
#define GPU_IT_FLAGS_READ (1 << 1)
/// FLAGS form GPU are being read righ now. 0 if is not reading in current moment
#define GPU_IT_FLAGS_READING (1 << 2)



// 
static ft80x_engine_it_TypeDef _thisData ;



// ######### FORWARD DECLARATION OF SHARED IT API ##########
// Defined in ft80x_it_api.c
bool ft80x_it_api_isSending( void ) ;


// ######### FORWARD DECLARATION OF LOCAL FUNCTIONS ##########
static FT80xTask_TypeDef * _getTask( const uint16_t id );







// ######### PUBLIC API #########


// this is the IT rountine -> must be called in interrupt procedure
void ft80x_gpu_eng_it_rountine(void)
{
    // set flag that it from the GPU IT has just arisen
    _thisData.m_gpuIT_flag |= GPU_IT_NEED_READ ;
}


// call this function in loop, it handles all task switching and other staff
void ft80x_gpu_eng_it_looper(void)
{
    FT80xTask_TypeDef * curr_task = _thisData.m_currTask ;
    
    // do nothing if no task is active
    if ( NULL == curr_task )
        return ;
    
    
    // read if communication is in progress ->from it_api 
    bool comm_in_progress = ft80x_it_api_isSending();
    
    // check for interrupts
    if ( false == comm_in_progress )
    {
        if ( _thisData.m_gpuIT_flag & GPU_IT_NEED_READ )
        {
            _thisData.m_gpuIT_flag &= ~GPU_IT_NEED_READ ;
            _thisData.m_gpuIT_flag |= GPU_IT_FLAGS_READING ;
            ///
            // todo -> read data from GPU register
            ///
            _thisData.m_gpuIT_flag &= ~GPU_IT_FLAGS_READING ;
            _thisData.m_gpuIT_flag |= GPU_IT_FLAGS_READ ;
        }
    }
    
    
    // handle task management
    switch( _thisData.m_currTaskState )
    {
        case New :
            // set state to Painting
            _thisData.m_currTaskState = Painting ;
            // call task's painting function
            _thisData.m_currTask->mfp_painting() ;
            break;
        case Painting :
            // todo -> wait for CMD processed IT -> based on IT from GPU
            if ( false == comm_in_progress )
            {
                // switch to Running if communication was finished
                _thisData.m_currTaskState = Running ;
            }
            break;
        case Running :
            // call running task
            _thisData.m_currTask->mfp_doing( curr_task->mp_shared_data ) ;
            break ;
    }
}



void ft80x_gpu_eng_it_init( void )
{
    _thisData.m_gpu_itflags = 0;
    _thisData.m_gpuIT_flag = 0 ;
    _thisData.m_currTask = NULL ;
    _thisData.m_taskListHead = NULL ;
}


bool ft80x_gpu_eng_it_reg_task( FT80xTask_TypeDef * const pTask )
{
    // check if that task already exist
    FT80xTask_TypeDef * task_ptr ;
    
    // handle the head
    if ( NULL == _thisData.m_taskListHead )
    {
        _thisData.m_taskListHead = pTask ;
        return true ;
    }
    else
    {
        task_ptr = _thisData.m_taskListHead ;
    }
    
    
    do
    {
        if ( pTask->m_id == task_ptr->m_id )
            return false ;
        
        if ( NULL != (task_ptr->next) )
            task_ptr = task_ptr->next ;
        else
            break; 
    }
    while  (  task_ptr != NULL );
    
    
    // add the task
    task_ptr->next = pTask ;
    return true;
}



void ft80x_gpu_eng_it_setActiveTask( const uint16_t task_id )
{
    // reset internal state
    _thisData.m_currTaskState = New ;
    _thisData.m_gpuIT_flag = 0 ;
    _thisData.m_gpu_itflags = 0 ;

    // set active state
    _thisData.m_currTask = _getTask( task_id );
}










// ######### LOCAL (STATIC) - HELPER FUNCTIONS #########

static FT80xTask_TypeDef * _getTask( const uint16_t id )
{
    FT80xTask_TypeDef * task_ptr = _thisData.m_taskListHead ;
    
    while ( NULL != task_ptr )
    {
        if ( id == task_ptr->m_id )
            break ;
        else
            task_ptr = task_ptr->next ;
    }
    
    return task_ptr ;
}
