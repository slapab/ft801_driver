#include "stm32f4xx.h"



// Configure the exti pins


void ft801_gpu_exti_conf(void)
{
    // PB4
    
    // ######### ENABLE clock for this GPIO ##########
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;
    
    // ######### ENABLE clock for SYSCFG #############
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN ;
    
    
    // ######### Configure the gpio ###################
    GPIOB->MODER &= ~(GPIO_MODER_MODER4_0 | GPIO_MODER_MODER4_1 ); // as input
    
    // pull up
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR4_0) ;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR4_1) ;
    
    
    
    
    
    // ######### CONFIGURE SYS TO ENABLE GPIO with EXTI #######################
    // syscfg_exticr2
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB ;
    
    
    
    
    // ######### CONFIGURE EXTI registers ##########
    EXTI->IMR |= EXTI_IMR_MR4 ;  // ennable interrupt
    EXTI->FTSR |= EXTI_FTSR_TR4; // falling edge
    
}
