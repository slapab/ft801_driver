#ifndef _FT801_API_DISPLAYLIST_H_
#define _FT801_API_DISPLAYLIST_H_

/*
*   API for using DL ( display list ) in blocking way
*/


/**
*   This is using for setting the start addres in the GPU_DL memory.
*   After calling this function each calls to the ... will send 4 data bytes
*   in the next location in GPU_DL.
*
*   @param start_addr   the addres in GPU_DL memory
*   @param *buff        pointer to the buffer array where will be stored all
                        DL commands.
*   @param buff_size    the size of buffer #buff. If this value is < 0 then
*                       calls to this function just reset the internal state.
*/
void ft801_api_dl_prepare( uint32_t start_addr, uint32_t * buff, int32_t buff_size ) ;

/**
*   This function appends DL command to the buffer.

*   @return false if there is no place in the buffer
*/
bool ft801_api_dl_append( const uint32_t data );

/**
*   This function sends data from internal buffer to the GPU.
*   
*   @retval 0 if data was successfully sent
*   @retval 1 if buffer was empty and nothing was sent
*   @retval 2 if buffer was not specified
*/
uint32_t ft801_api_dl_flush( void ) ;
  
#endif
