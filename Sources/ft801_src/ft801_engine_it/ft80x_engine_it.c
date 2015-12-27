
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
/// If 1 that means that received IT with information that processing CMD was just 
/// completed and can send new commands
#define GPU_IT_FLAGS_CMDREADY ( 1 << 3 )

// 
static ft80x_engine_it_TypeDef _thisData ;



// ######### FORWARD DECLARATION OF SHARED IT API ##########
// Defined in ft80x_it_api.c
bool ft80x_it_api_isSending( void ) ;
uint16_t ft80x_spi_read_16bits( const uint32_t addr ) ;



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
    
    // check for interrupts, but only when is no ongoing transmission
    if ( false == comm_in_progress )
    {
        if ( _thisData.m_gpuIT_flag & GPU_IT_NEED_READ )
        {
            // disable flag that need to read IT FLAGS from GPU
            _thisData.m_gpuIT_flag &= ~GPU_IT_NEED_READ ;
            // READING IT FLAGS is on progress
            //_thisData.m_gpuIT_flag |= GPU_IT_FLAGS_READING ;
            
            // read flags from the GPU and store in local variable
            uint16_t tmp = ft80x_spi_read_16bits(REG_INT_FLAGS) ;
            _thisData.m_gpu_itflags = (uint8_t)tmp ; // MSB because it is Little-endian
            
            
            // READING IT was completed
            //_thisData.m_gpuIT_flag &= ~GPU_IT_FLAGS_READING ;
            // IT Flags from GPU has been read
            _thisData.m_gpuIT_flag |= GPU_IT_FLAGS_READ ;
        }
    }
    
    
    // HANDLE HERE IT FLAGS
    // If flags was read from GPU then send it to the user function
    if ( _thisData.m_gpuIT_flag & GPU_IT_FLAGS_READ )
    {
        // Clear information -> fire this code only once for each interrupt
        _thisData.m_gpuIT_flag &= ~GPU_IT_FLAGS_READ ;
        
        
        // check if CMD empty interrupt was set
        if ( _thisData.m_gpu_itflags & FT_INT_CMDEMPTY )
        {
            _thisData.m_gpuIT_flag |= GPU_IT_FLAGS_CMDREADY ;
        }
        
 
        // Call user function to handle interrupts
        if ( NULL != curr_task->mfp_gpu_it )
        {
            curr_task->mfp_gpu_it(_thisData.m_gpu_itflags, curr_task->mp_shared_data);
        }
        
        // clear IT
    }
    
    
    // handle task management
    switch( _thisData.m_currTaskState )
    {
        case New :
            // set state to Painting
            _thisData.m_currTaskState = Painting ;
            // clear flag CMDREADY -> commands will be sending right now
            _thisData.m_gpuIT_flag &= ~GPU_IT_FLAGS_CMDREADY ;
            // call task's painting function
            _thisData.m_currTask->mfp_painting(curr_task->mp_shared_data) ;
            break;
        case Painting :
            if ( _thisData.m_gpuIT_flag & GPU_IT_FLAGS_CMDREADY )
            {
                // switch to Running if communication was finished and if 
                // GPU finished processing the commands -> it just printed the screen
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
    
    // reset flags but keep the IT flag -> the it flags from GPU must be read
    _thisData.m_gpuIT_flag = _thisData.m_gpuIT_flag & GPU_IT_NEED_READ ;
    
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
