#include "stm32f4xx.h"

#include "stdbool.h"	// using bool type
#include "ft801_gpu.h"


#define GPIO_SPI_GPU GPIOE
#define GPIO_SPI_GPU_RCC_CLOCK_EN RCC_AHB1ENR_GPIOEEN // Which GPIO need to turn clock
#define PIN_SPI_GPU_SCK		2
#define PIN_SPI_GPU_NSS 	4
#define PIN_SPI_GPU_MISO	5
#define PIN_SPI_GPU_MOSI	6

#define AFR5_PIN(x) (uint32_t)((1 << ((x)*4 + 0) )|(1 <<((x)*4 + 2) ))


// Defines for spi
#define SPI_GPU SPI4


static void gpu_init_spi_gpio()
{
	
	// Enable clock gating for GPIO
	RCC->AHB1ENR |= GPIO_SPI_GPU_RCC_CLOCK_EN ;
	
	// sets the alternate function -SPI4- for gpioe pins
	GPIO_SPI_GPU->MODER |= GPIO_MODER_MODER2_1	/* SCK */		
								| GPIO_MODER_MODER4_1 /* NSS */
								| GPIO_MODER_MODER5_1	/* MISO */
								| GPIO_MODER_MODER6_1; /* MOSI */
	
	// select AF number AFR5 for SPI4 on PortE
	GPIO_SPI_GPU->AFR[0] |=  AFR5_PIN(PIN_SPI_GPU_SCK) /* SCK */
                            | AFR5_PIN(PIN_SPI_GPU_NSS)	/* NSS */
                            | AFR5_PIN(PIN_SPI_GPU_MISO)	/* MISO */
                            | AFR5_PIN(PIN_SPI_GPU_MOSI); /* MOSI */
	
	
	// sets speed for gpio _0 -> medium speed
	GPIO_SPI_GPU->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2_0
                            | GPIO_OSPEEDER_OSPEEDR4_0
                            | GPIO_OSPEEDER_OSPEEDR5_0
                            | GPIO_OSPEEDER_OSPEEDR6_0
                            ;

}

void gpu_init_spi(void)
{
	// Init gpio for spi
	gpu_init_spi_gpio() ;
	
	
	// Init SPI for gpu: 
	// select clock
	// CPOL and CPHA -> mode 0 is required
	// DFF, 8 data frame format
	// MSB first
	// NSS output mode -> configuration as master
	
	
	// Enable clock gating for SPI
	RCC->APB2ENR |= RCC_APB2ENR_SPI4EN ;
	
	SPI_GPU->CR1 |= SPI_CR1_BR_0		// prescaler */
					| SPI_CR1_MSTR	    // master configuration
					;
    
    SPI_GPU->CR2 |= SPI_CR2_RXNEIE      // recived data interrupt enable
                   | SPI_CR2_TXEIE      // tx empty interrupt enable
                   | SPI_CR2_SSOE       // NSS pin is driven by hardware
                   ;
}


// blocking function!!!!
inline static void _ft801_spi_wait_txempty()
{
    volatile uint32_t i = 0;
    while(!READ_BIT(SPI_GPU->SR, SPI_SR_TXE) ){}
}

inline static void _ft801_spi_clear_rx_flag()
{
    if ( SPI_GPU->SR & SPI_SR_RXNE )
    {
        uint8_t t = SPI_GPU->DR;
    }
}

inline static void _ft801_spi_wait_for_rx()
{   
    volatile uint32_t i ;
    while((SPI_GPU->SR & SPI_SR_RXNE) == 0) {}
}

inline static void _ft801_spi_wait_for_notBusy()
{
    //volatile uint32_t i = 0;
    while( READ_BIT(SPI_GPU->SR, SPI_SR_BSY) ){}
}



inline static void _ft801_spi_write_tx( const uint8_t data )
{
    _ft801_spi_wait_txempty();
    SPI_GPU->DR = data ;
}



// just push only three bytes
// This is not the little-endian
static void _ft801_spi_write_address( const uint32_t addr ) 
{

    for (int i = 2 ; i >= 0 ; --i )
    {
        // Wait for tx buffer empty
        _ft801_spi_wait_txempty();
        SPI_GPU->DR  = (uint8_t)((addr >> (i<<3))) ;    // i<<3 means i*8
    }
}


// possible hang on disabling
void ft801_spi_enable( const bool enabled )
{
	if ( enabled )
	{
		SPI_GPU->CR1 |= SPI_CR1_SPE;
	}
	else
	{
        // hang till it will be not busy and tx buffer is empty
//        while( (SPI_GPU->SR & SPI_SR_BSY) && !(SPI_GPU->SR & SPI_SR_TXE) );
        _ft801_spi_wait_txempty();
        _ft801_spi_wait_for_notBusy();
		SPI_GPU->CR1 &= ~(SPI_CR1_SPE);
	}
}

/** Write command -> only write -> all readed data are not relevant
*   Writing only three bytes ( in that order ) MSB+2, MSB+1, MSB    
*/
void ft801_spi_host_cmd( uint32_t cmd )
{   
    // prepare cmd arg as host command
//    cmd &=  ~(1 << 23) ;
//    cmd |= (1 << 22);
//    cmd &= 0x00FF0000 ;
    
    ft801_spi_enable(true) ;
    _ft801_spi_write_address( cmd );
    ft801_spi_enable(false) ;
}

void spi_write_stream( uint8_t * const buff, const uint32_t len )
{
    ft801_spi_enable(true);
    
    for ( uint32_t i = 0 ; i < len ; ++i )
    {
        _ft801_spi_wait_txempty();
        SPI_GPU->DR = buff[i];
    }
    
    _ft801_spi_wait_for_notBusy();
    
    ft801_spi_enable(false);
}


// little-endian
void ft801_spi_mem_wrStream( const uint32_t addr,
                            uint32_t * pBuff,
                            const uint32_t len )
{
    ft801_spi_enable(true);
    
    // write the address
    _ft801_spi_write_address( addr | 0x00800000 ) ;
    
    for ( uint32_t item = 0 ; item < len ; ++item )
    {
        // Write the data - little endian
        for ( uint32_t i = 0 ; i < 4; ++i )
        {
            _ft801_spi_wait_txempty();
            SPI_GPU->DR = (uint8_t)( pBuff[item] >> (i<<3) ); // i<<3 generates, 0, 8, 16, 24 bit shifting
        }
    }
    
    ft801_spi_enable(false);
}

// little-endian
void ft801_spi_mem_wr32( const uint32_t addr,
                         const uint32_t data)
{

    ft801_spi_enable(true);
    
    // write the address
    _ft801_spi_write_address( addr | 0x00800000 ) ;
    
    // Write the data - little endian
    for ( uint32_t i = 0 ; i < 4; ++i )
    {
        _ft801_spi_wait_txempty();
        SPI_GPU->DR = (uint8_t)( data >> (i<<3) ); // i<<3 generates, 0, 8, 16, 24 bit shifting
    }
    
    ft801_spi_enable(false);
}                           



void ft801_spi_mem_wr16( const uint32_t addr,
                         const uint16_t data )
{

    ft801_spi_enable(true);
    
    // write the address
    _ft801_spi_write_address( addr | 0x00800000 ) ;
    
    // Write the data - little endian
    for ( uint32_t i = 0 ; i < 2 ; ++i )
    {
        _ft801_spi_wait_txempty();
        SPI_GPU->DR = (uint8_t)( data >> (i<<3) );
    }
    
    ft801_spi_enable(false);
}


void ft801_spi_mem_wr8( const uint32_t addr,
                        const uint8_t data )
{

    ft801_spi_enable(true);
    
    // write the address
        _ft801_spi_write_address( addr | 0x00800000);
    
    // write the data
    _ft801_spi_wait_txempty();
    SPI_GPU->DR = (uint8_t)data;
    
    ft801_spi_enable(false);
}



uint16_t ft801_spi_rd16( uint32_t addr )
{
    uint16_t tmp_out;
    
    // be sure that this is memory-read command style
    addr = addr & 0xFF3FFFFF ;
    
    ft801_spi_enable(true);
   
    // write the address
    _ft801_spi_write_address(addr);
    _ft801_spi_wait_txempty();
    
    // write dummy byte - and read dummy byte
    SPI_GPU->DR = 0x07 ;
    _ft801_spi_wait_txempty();
    _ft801_spi_wait_for_rx();
    tmp_out = SPI_GPU->DR;
    
    // read proper value - little endian
    tmp_out = 0;
    for ( uint32_t i = 0; i < 2; ++i )
    {
        SPI_GPU->DR = 0x07 ;
        
        // get the byte
         _ft801_spi_wait_txempty();
         _ft801_spi_wait_for_rx();
        tmp_out |= ((uint16_t)SPI_GPU->DR << (i<<3)) ;
    }

    ft801_spi_enable(false);
    
    return tmp_out ;
}
                         
                         
uint8_t ft801_spi_rd8( uint32_t addr )
{
    // be sure that this is memory-read command style
    addr = addr & 0xFF3FFFFF ;
    
    // Enable SPI
    ft801_spi_enable(true);

    // write the address
    _ft801_spi_write_address(addr);
    _ft801_spi_wait_txempty();
    
    uint8_t tmp_r = 0;
    for (uint32_t i = 0 ; i < 2 ; ++i) // get the dummy and proper byte
    {
        SPI_GPU->DR = 0x01 ;
        
        _ft801_spi_wait_txempty();

        if ( i == 1 )
            _ft801_spi_wait_for_rx() ;        
        
        tmp_r = SPI_GPU->DR ;
    }
        
    // Disable SPI
    ft801_spi_enable(false);    
    
    return tmp_r ;
}



uint32_t ft801_spi_rd32( uint32_t addr )
{
    uint32_t tmp_out;
    
    // be sure that this is memory-read command style
    addr = addr & 0xFF3FFFFF ;
    
    ft801_spi_enable(true);
   
    // write the address
    _ft801_spi_write_address(addr);
    _ft801_spi_wait_txempty();
    
    // write dummy byte - and read dummy byte
    SPI_GPU->DR = 0x07 ;
    _ft801_spi_wait_txempty();
    _ft801_spi_wait_for_rx();
    tmp_out = SPI_GPU->DR;
    
    // read proper value - little endian
    tmp_out = 0;
    for ( uint32_t i = 0; i < 4; ++i )
    {
        SPI_GPU->DR = 0x07 ;
        
        // get the byte
         _ft801_spi_wait_txempty();
         _ft801_spi_wait_for_rx();
        tmp_out |= ((uint16_t)SPI_GPU->DR << (i<<3)) ;
    }

    ft801_spi_enable(false);
    
    return tmp_out ;
}




