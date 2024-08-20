#include <stdio.h>
#include <system.h>
#include <sys/alt_irq.h>
#include <alt_types.h>
#include <altera_avalon_timer_regs.h>
#include <stdlib.h>

// Function Declarations
void init_timer_interrupt(void);
static void timer_isr(void *context, alt_u32 id);

volatile alt_u32  * time= (alt_u32 *) 0x21030;
alt_u32  Display[10]={64,121,36,48,25,18,2,120,0,16};

int main(void) {
    printf("Hello from Nios II!\n");
    printf("This is an interrupt example using an interval timer with interrupts.");

    // Enable the timer
    init_timer_interrupt();

    while(1) {
    }

    return 0;
}

void init_timer_interrupt(void) {
    // Register the ISR with HAL
    alt_ic_isr_register(TIMER_IRQ_INTERRUPT_CONTROLLER_ID, TIMER_IRQ, (void*)timer_isr, NULL, 0x0);

    // Start the timer
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                     ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                     ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);
}

static void timer_isr(void *context, alt_u32 id) {
    static short seconds = 55;// Clear the interrupt
    static int minutes=0;
    static int hrs=0;
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);// Clear the interrupt
    // Do something
    //*time=Display[seconds%10];
    alt_u32 calculo=(((Display[seconds%10])&0xFFFFFFFF)+((Display[seconds/10]<<7)&0xFFFFFFFF)+((Display[minutes%10]<<14)&0xFFFFFFFF)+((Display[minutes/10]<<21)&0xFFFFFFFF));
    *time=calculo;
    printf("\nTimer Expired: %d", seconds++);

    if(seconds>=60){
      	minutes++;
      	printf("\nBOLAS: %d", minutes);
      	seconds=0;
    }if(minutes==60){
      	hrs++;
      	minutes=0;
    }
    hrs=hrs%24;





}
