#include "stm32f4xx.h"

#include "stdbool.h"	// using bool type


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

static void _ft801_spi_wait_for_rx()
{   
    volatile uint32_t a ;
    while((SPI_GPU->SR & SPI_SR_RXNE) == 0) {++a;}
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
static void _ft801_spi_write_3bytes( const uint32_t data ) 
{
    uint8_t tmp = 0;
    for (int i = 2 ; i >= 0 ; --i ) 
    {
        // Wait for tx buffer empty
        _ft801_spi_wait_txempty();
        tmp = (uint8_t)((data >> (8*i))) ;
        SPI_GPU->DR = tmp;
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
        while( (SPI_GPU->SR & SPI_SR_BSY) && !(SPI_GPU->SR & SPI_SR_TXE) );
		SPI_GPU->CR1 &= ~(SPI_CR1_SPE);
	}
}

/** Write command -> only write -> all readed data are not relevant
*   Writing only three bytes ( in that order ) MSB+2, MSB+1, MSB    
*/
void ft801_spi_hcmd_write( const uint32_t cmd )
{
    uint8_t tmp = 0 ;
    
    ft801_spi_enable(true) ;
    _ft801_spi_write_3bytes( cmd );
    ft801_spi_enable(false) ;
}


void ft801_spi_mem_wr16( const uint32_t addr,
                         const uint16_t data,
                         const bool one_shot )
{
    if ( one_shot )
        ft801_spi_enable(true);
    
    // write the address
    _ft801_spi_write_3bytes( addr ) ;
    
    // Write the data
    for ( int i = 1 ; i >= 0 ; --i )
    {
        _ft801_spi_wait_txempty();
        SPI_GPU->DR = (uint8_t)( data >> (i*8) );
    }
    
    if ( one_shot )
        ft801_spi_enable(false);
}

uint16_t ft801_spi_mem_rd16( const uint32_t addr )

{
    ft801_spi_enable(true);
    
    uint8_t tmp = 0;
    // write address
    for (int i = 2; i >= 0; --i ) 
    {
        // Wait for tx buffer empty
        _ft801_spi_wait_txempty();
        tmp = (uint8_t)((addr >> (8*i))) ;
        SPI_GPU->DR = tmp;
    }
    
    // Write dummy bytes to get the out
    // First byte is a dummy byte, but te second and third are the valid
    uint16_t data = 0;
    for ( int i = 0 ; i < 3 ; ++i ) 
    {
        _ft801_spi_wait_txempty();
//        if ( i == 1 ) // get MSB byte
//        {
//            while ( !(SPI_GPU->SR & SPI_SR_RXNE) ) ;
//            data |= (uint16_t)((SPI_GPU->DR & 0x00FF) << 8) ;
//        }
        if ( i == 2 ) // get LSB byte
        {
            while ( !(SPI_GPU->SR & SPI_SR_RXNE) ) ;
            data |= SPI_GPU->DR & 0x00FF ;
        }
        else if ( i < 2 ) // read dummy byte
        {
            while ( !(SPI_GPU->SR & SPI_SR_RXNE) ) ;
            tmp = SPI_GPU->DR ;
        }
       
        SPI_GPU->DR = 0x01 ;    // write dummy byte
    }
    
    // Get the LSB byte:
    while ( !(SPI_GPU->SR & SPI_SR_RXNE) ) ;
    data |= (uint16_t)((SPI_GPU->DR & 0x00FF) << 8) ;

    
    ft801_spi_enable(false);
    
    return data ;
}
                         
                         
void gpu_spi_send( uint32_t reg )
{
    //reg &= 0x00FFFFFF ; // make read command
    
    _ft801_spi_write_3bytes( reg );
    
    // Dummy byte is first, but te second is the required byte
    for (int i =  0 ; i < 2 ; ++i ) 
    {
        // Wait for tx buffer empty
       _ft801_spi_wait_txempty();
        SPI_GPU->DR = 0x00;
    }
}



uint8_t ft801_spi_rd8( const uint32_t reg )
{
    // Enable SPI
    ft801_spi_enable(true);

    _ft801_spi_wait_txempty();
    
    uint8_t tmp_r = 0;
    for (int i = 2, j = 0; j < 5 ; --i, ++j) 
    {
        if ( i >= 0 )   // write only memory address
            SPI_GPU->DR = (uint8_t)((reg >> (8*i))) ;
        else
            SPI_GPU->DR = 0x01 ;
        
        _ft801_spi_wait_txempty();
        _ft801_spi_wait_for_rx() ;
        tmp_r = SPI_GPU->DR ;
        
    }
        
    // Disable SPI
    ft801_spi_enable(false);    
    
    return tmp_r ;
}






