#include <stdio.h>
#include <system.h>
#include <stdbool.h>
#include <sys/alt_irq.h>
#include <alt_types.h>
#include <altera_avalon_timer_regs.h>
#include <altera_avalon_pio_regs.h>
#include <stdlib.h>

// Function Declarations
void init_timer_interrupt(void);
static void timer_isr(void *context, alt_u32 id);
static void pio1_isr(void* context, alt_u32 id);
static void pio2_isr(void* context, alt_u32 id);

volatile short  * switches = (short *) 0x21020;
volatile short  * buttons = (short *) 0x21030;
volatile alt_u32  * leds= (alt_u32 *) 0x21040;

alt_u32  Display[10]={64,121,36,48,25,18,2,120,0,16};

static int seconds = 55;
static int minutes=59;
static int hrs=23;
static int sum=1;
static int min_mod=0;
static int alarm_disp=3;


bool time_mod=false;
bool alarm_mod=false;
bool alarm_on=false;
bool alarm_en=false;

static int alarm_minutes=00;
static int alarm_hrs=17;




int main(void) {
    printf("Hello from Nios II!\n");

    // Enable the timer
    init_interruptions();

    while(1) {
    }

    return 0;
}

// ISR para PIO1
static void pio1_isr(void* context, alt_u32 id) {
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_1_BASE, 0);
    switch((*(buttons))){
    	case 11://Add
    		if(time_mod){//Time modifying
    			if(min_mod<=1){//Add on minutes
    				minutes+=sum;
    				minutes=minutes%60;
    			}else{//Add on hours
    				if(hrs+sum>=24&&min_mod==3){
    					hrs=hrs%10;
    				}else{
    					hrs+=sum;
    					hrs=10*((hrs/10)%3)+hrs%10;
        				hrs=hrs%24;
    				}
    			}
    		}else{//Alarm modifying
    			if(min_mod<=1){//Add on alarm minutes
    				alarm_minutes+=sum;
    				alarm_minutes=alarm_minutes%60;
    			}else{//Add on alarm hours
    				alarm_hrs+=sum;
    				alarm_hrs=10*((alarm_hrs/10)%3)+alarm_hrs%10;
    				alarm_hrs=alarm_hrs%24;
    			}
    		}
    		break;
    	case 7://Sub
    		if(time_mod){//Time modifying
    			if(min_mod<=1){//Add on minutes
    		    	minutes-=sum;
    		    	minutes=minutes%60;
    		    	if(minutes<0){
    		    		minutes=60+minutes;
    		    	}
    		    }else{//Add on hours
    		    	if(((hrs-sum)<0)){
						if(min_mod==3&&((hrs%10)<=3)){
							hrs=20+(hrs%10);
						}else if(min_mod==3&&((hrs%10)<=9)){
							hrs=10+(hrs%10);
    		    		}else{
							hrs=23;
						}
    		    	}else{
    		    		hrs-=sum;
    		    	}

	    		}
    		}else{//Alarm modifying
    			if(min_mod<=1){//Add on alarm minutes
     				alarm_minutes-=sum;
       				alarm_minutes=alarm_minutes%60;
       				if(alarm_minutes<0){
       					alarm_minutes=60+alarm_minutes;
       				}
       			}else{//Add on alarm hours
       				alarm_hrs-=sum;
       				alarm_hrs=alarm_hrs%24;
       				if(alarm_hrs<0){
       					alarm_hrs=60+alarm_hrs;
       				}
       			}
       		}
       		break;
    	case 13://Change digit that is being modified
    		sum=11-sum;
    		min_mod++;
    		min_mod=min_mod%4;
    		break;
    	case 14://Turn off alarm;
    		alarm_on=false;
    		alarm_disp=3;
    		break;
    	default:
    		;
    }
}

// ISR para PIO2
static void pio2_isr(void* context, alt_u32 id) {
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_2_BASE, 0);//Reinicia la alarma
    min_mod=0;
    sum=1;
    time_mod=(((*(switches))&0x1)==0x1);//Define si el tiempo esta siendo modificado
    alarm_mod=(((*(switches))&0x2)==0x2);//Define si la alarma esta siendo modificada
    alarm_en=(((*(switches))&0x4)==0x4);//Define si la alarma debería sonar o no
}

void init_interruptions(void) {
	//Timer Init
    alt_ic_isr_register(TIMER_IRQ_INTERRUPT_CONTROLLER_ID, TIMER_IRQ, (void*)timer_isr, NULL, 0x0);

    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                     ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                     ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);

    //Buttons Init
    alt_ic_isr_register(PIO_1_IRQ_INTERRUPT_CONTROLLER_ID, PIO_1_IRQ, (void*)pio1_isr, NULL, 0x0);

    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_1_BASE, 0xF);  // Habilitar interrupciones en PIO_1 (4 bits)


    //Switches Init
    alt_ic_isr_register(PIO_2_IRQ_INTERRUPT_CONTROLLER_ID, PIO_2_IRQ, (void*)pio2_isr, NULL, 0x0);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_2_BASE, 0xF);  // Habilitar interrupciones en PIO_2 (4 bits)

}

static void timer_isr(void *context, alt_u32 id) {
	static bool blinker=true;
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);// Clear the interrupt
	if((!time_mod)&&(!alarm_mod)){//Time nor alarm is being modified
		//Put on the display
		alt_u32 time_calc=(((Display[minutes%10])&0xFFFFFFFF)+((Display[minutes/10]<<7)&0xFFFFFFFF)+((Display[hrs%10]<<14)&0xFFFFFFFF)+((Display[hrs/10]<<21)&0xFFFFFFFF))+((blinker&&alarm_on)<<28)+(alarm_en<<29)+(alarm_disp<<30);
	    *leds=time_calc;

	    //Time management
		seconds++;
		if(seconds>=60){
			minutes++;
			seconds=0;
		}if(minutes==60){
			hrs++;
			minutes=minutes%60;
		}hrs=hrs%24;

		//Alarm Management
		if(((seconds==3)&&(minutes==alarm_minutes)&&(hrs==alarm_hrs))&&(alarm_en)){
			alarm_on=true;
		}if(alarm_on){
			alarm_disp=1+blinker;
			blinker=!blinker;
		}

	}else if(time_mod){
		if(blinker){
			alt_u32 time_calc=(((Display[minutes%10])&0xFFFFFFFF)+((Display[minutes/10]<<7)&0xFFFFFFFF)+((Display[hrs%10]<<14)&0xFFFFFFFF)+((Display[hrs/10]<<21)&0xFFFFFFFF))+((blinker&&alarm_on)<<28)+(alarm_en<<29)+(alarm_disp<<30);
			*leds=time_calc;
		}else{
			alt_u32 time_calc=((127&0xFFFFFFFF)+((127<<7)&0xFFFFFFFF)+((127<<14)&0xFFFFFFFF)+((127<<21)&0xFFFFFFFF))+((blinker&&alarm_on)<<28)+(alarm_en<<29)+(alarm_disp<<30);
			*leds=time_calc;
		}
		blinker=!blinker;
	}else if(alarm_mod){
		if(blinker){
			alt_u32 time_calc=(((Display[alarm_minutes%10])&0xFFFFFFFF)+((Display[alarm_minutes/10]<<7)&0xFFFFFFFF)+((Display[alarm_hrs%10]<<14)&0xFFFFFFFF)+((Display[alarm_hrs/10]<<21)&0xFFFFFFFF))+((blinker&&alarm_on)<<28)+(alarm_en<<29)+(alarm_disp<<30);
			*leds=time_calc;
		}else{
			alt_u32 time_calc=((127&0xFFFFFFFF)+((127<<7)&0xFFFFFFFF)+((127<<14)&0xFFFFFFFF)+((127<<21)&0xFFFFFFFF))+((blinker&&alarm_on)<<28)+(alarm_en<<29)+(alarm_disp<<30);
			*leds=time_calc;
		}
		blinker=!blinker;
	}
}
