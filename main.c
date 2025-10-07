#include "tm4c123gh6pm.h"
#include <stdint.h>

#define SW1     (1U << 4)
#define SW2     (1U << 0)
#define SWITCH_MASK (SW1 | SW2)

volatile int duty_percent = 50;

void portf_init(void);
void timer0_pwm_init(void);

int main(void) {
    portf_init();
    timer0_pwm_init();

    while (1) {
        // Do nothing, everything handled in ISR
    }
}


void portf_init(void) {
    SYSCTL_RCGCGPIO_R |= (1 << 5);       // Enable clock for Port F
    while ((SYSCTL_PRGPIO_R & (1 << 5)) == 0) {}  // Wait

    GPIO_PORTF_LOCK_R = 0x4C4F434B;      // Unlock PF0
    GPIO_PORTF_CR_R |= SWITCH_MASK;      // Commit changes

    GPIO_PORTF_DIR_R &= ~SWITCH_MASK;    // PF0, PF4 as input
    GPIO_PORTF_DEN_R |= SWITCH_MASK;     // Digital enable
    GPIO_PORTF_PUR_R |= SWITCH_MASK;     // Pull-up resistors

    // Interrupt config
    GPIO_PORTF_IS_R &= ~SWITCH_MASK;     // Edge-sensitive
    GPIO_PORTF_IBE_R &= ~SWITCH_MASK;    // Not both edges
    GPIO_PORTF_IEV_R &= ~SWITCH_MASK;    // Falling edge
    GPIO_PORTF_ICR_R = SWITCH_MASK;      // Clear any prior interrupt
    GPIO_PORTF_IM_R |= SWITCH_MASK;      // Unmask interrupt

    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF3FFFFF) | (3 << 21); // Priority 3
    NVIC_EN0_R |= (1 << 30);             // Enable interrupt in NVIC
}

// ===== Init Timer0A for PWM on PB6 =====
void timer0_pwm_init(void) {
    SYSCTL_RCGCGPIO_R |= (1 << 1);      // Enable clock for Port B
    while ((SYSCTL_PRGPIO_R & (1 << 1)) == 0) {}

    SYSCTL_RCGCTIMER_R |= (1 << 0);     // Enable clock for Timer0
    while ((SYSCTL_PRTIMER_R & (1 << 0)) == 0) {}

    // Configure PB6 for T0CCP0
    GPIO_PORTB_AFSEL_R |= (1 << 6);
    GPIO_PORTB_PCTL_R &= ~(0xF << 24);
    GPIO_PORTB_PCTL_R |= (0x7 << 24);   // PB6 = T0CCP0
    GPIO_PORTB_DEN_R |= (1 << 6);

    TIMER0_CTL_R &= ~1;                 // Disable Timer0A
    TIMER0_CFG_R = 0x04;                 // 16-bit mode
    TIMER0_TAMR_R = 0x0A;                // PWM mode, periodic
    TIMER0_TAILR_R = 1600 - 1;           // Load for 50 kHz @ 80MHz
    TIMER0_TAMATCHR_R = 800;             // 50% duty cycle
    TIMER0_CTL_R |= (1 << 0) | (1 << 8); // Enable Timer0A + CCP output
}

// ===== Port F ISR =====
void GPIOF_Handler(void) {
    uint32_t status = GPIO_PORTF_MIS_R;

    if (status & SW1) {  // PF4 pressed
        if (duty_percent > 5) duty_percent -= 5;
        GPIO_PORTF_ICR_R = SW1;
    }
    if (status & SW2) {  // PF0 pressed
        if (duty_percent < 95) duty_percent += 5;
        GPIO_PORTF_ICR_R = SW2;
    }

    // Update Timer0A match value
    uint32_t load = 1600;
    TIMER0_TAMATCHR_R = load - (duty_percent * load) / 100;
}
