#include "tm4c123gh6pm.h"
#include <stdint.h>

#define STCTRL_R (*(volatile uint32_t *)0xE000E010)
#define STRELOAD_R (*(volatile uint32_t *)0xE000E014)
#define STCURRENT_R (*(volatile uint32_t *)0xE000E018)



#define SWITCH_MASK 0x11

#define RED_LED (1<<1)
#define BLUE_LED (1<<2)
#define GREEN_LED (1<<3)
#define SW2 (1<<4)
#define SW1 (1<<0)
void portf_init();
int main(void)
{
    portf_init();


    GPIO_PORTF_IM_R |= 0;//Mask all interrupts

    GPIO_PORTF_IS_R &= ~SWITCH_MASK;
    GPIO_PORTF_IBE_R &= ~SWITCH_MASK;
    GPIO_PORTF_IEV_R &= ~SWITCH_MASK;

    NVIC_PRI7_R = (NVIC_PRI7_R &  0xF0FFFFFF) | (3 << 21);

    NVIC_EN0_R |= (1<< 30);

    GPIO_PORTF_ICR_R = SWITCH_MASK;
    GPIO_PORTF_IM_R |= SWITCH_MASK;
    STCURRENT_R = 0;
    STRELOAD_R = 500* 16000;
    STCTRL_R = 7;


	return 0;


}


void portf_init()
{
   SYSCTL_RCGC2_R |= 0x00000020;//Enable GPIO

   GPIO_PORTF_LOCK_R = 0x4C4F434B;
   GPIO_PORTF_CR_R = 0x1F;

   GPIO_PORTF_DEN_R |= 0x1F;
   GPIO_PORTF_DIR_R |= 0x0E;

   GPIO_PORTF_PUR_R = 0x11;

   GPIO_PORTF_DATA_R = 0x00;
}

void SysTickHandler(void)
{
    GPIO_PORTF_DATA_R ^= BLUE_LED;
}

void PortFHandler(void)
{
    if(GPIO_PORTF_MIS_R & SW1)
    {
        GPIO_PORTF_DATA_R ^= RED_LED;
        GPIO_PORTF_ICR_R  = SW1;
    }
    if(GPIO_PORTF_MIS_R & SW2)
    {
        GPIO_PORTF_DATA_R ^= GREEN_LED;
        GPIO_PORTF_ICR_R  = SW2;

    }
}
