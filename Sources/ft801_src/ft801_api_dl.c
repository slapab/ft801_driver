#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ft801_api_dl.h"

#include "spi.h" // todo -> remove this dependecy


struct ft801_dl_descr_t
{
    uint32_t  m_startAddr ;  // address in the FT801
    uint32_t* m_pBuff ;      // pinter to the beggining of buffer for DL commands
    uint32_t  m_buffSize ;   // size of buffer
    uint32_t* m_pNextPtr ;   // points to the next free element in the buffer
} ;



static struct ft801_dl_descr_t _dl_descr;



void ft801_api_dl_prepare(
    uint32_t start_addr,
    uint32_t * buff,
    int32_t buff_size )
{
    
    if ( buff_size > 0 )
    {
        _dl_descr.m_buffSize = (uint32_t)buff_size ;
        _dl_descr.m_pBuff = buff ;
    }
    
    _dl_descr.m_startAddr = start_addr ;
    _dl_descr.m_pNextPtr = _dl_descr.m_pBuff ;
}



bool ft801_api_dl_append( const uint32_t data )
{
    if( ( _dl_descr.m_pNextPtr - _dl_descr.m_pBuff ) > _dl_descr.m_buffSize )
    {
        return false ;
    }
    
    *(_dl_descr.m_pNextPtr) = data ;
    ++ _dl_descr.m_pNextPtr ;
    
    return true ;
}



// todo write the 'interface' to use calls to the interface -> user
// then need to 'implement' then the interface
uint32_t ft801_api_dl_flush( void )
{
    
    uint32_t maxSize = _dl_descr.m_pNextPtr - _dl_descr.m_pBuff ;
    
    if ( NULL == _dl_descr.m_pBuff )
        return 2 ;
    
    if ( 0 == maxSize )
        return 1 ;
    
    
    ft801_spi_mem_wrStream( _dl_descr.m_startAddr, _dl_descr.m_pBuff, maxSize ) ;
   
    return 0;
}



