/*----------------------------------------------------------------------------
Student ID: ec18446
Lab4: Activity 2:

		In this project:
    - the red and green LEDs flash on/off every 3s in a cycle, red or green is always on
		- pressing the button stops/restarts the flashing of the LEDs 
		- When flashing restarts, the LED that was alight when flashing stopped is light up again
		
		There are two threads:
       t_button: polls the button and signals t_greenLED
       t_flashLEDs: flashes the red and green LEDs on and off every 3 seconds in a cycle
										and/or it stops/restarts the flashing of the LEDs
										
		Macros added in gpio.h to implement STM transitions:
		- #define RED_ON (2)
		- #define RED_OFF (3)
		- #define GREEN_ON (4)
		- #define GREEN_OFF (5)
 *---------------------------------------------------------------------------*/
 
#include "cmsis_os2.h"
#include <MKL25Z4.h>
#include <stdbool.h>
#include "gpio.h"

osEventFlagsId_t evtFlags ; // event flags

/*---------------------------------------LAB 4: ACTIVITY 2-------------------------------------------*/
/*-----------------------------------------------------------------------------------
 *   Thread t_flashLEDs
 *       Flash red and green LEDs on/off in a cycle every 3 seconds
 *			 Or if button is pressed, it causes the flashing of the LEDs to stop/restart	
 *-----------------------------------------------------------------------------------*/
 osThreadId_t t_flashLEDs;        /* id of thread to flash red and green leds */
 
void flashLEDs (void *arg) {
    int ledState = RED_ON ; // Set initial state of STM, i.e. red LED is ON
    redLEDOnOff(LED_ON);    // Turn on red LED
	
    uint32_t flagStatus = 0U;	// it stores the result of the function call osEventFlagsWait
		uint32_t period = 3000;	// Flashing cycle period of 3 secs
	
    while (1) 
		{
				// Store the result of calling osEventFlagsWait()
				flagStatus = osEventFlagsWait(evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, period);
				// If the button event flag was NOT set within the timeout/period
				// then flash between red and green LEDs in a cycle every 3 secs until button is pressed
				if( flagStatus == osFlagsErrorTimeout) 
				{
					switch (ledState) 
					{
            case RED_ON:
                redLEDOnOff(LED_OFF);  
                greenLEDOnOff(LED_ON); 
                ledState = GREEN_ON ;	 
                break ;
            case GREEN_ON:
                redLEDOnOff(LED_ON); 
                greenLEDOnOff(LED_OFF);
                ledState = RED_ON;
                break ;
					}
				}
				// Else if button event flag was set within the timeout/period
				// then stop/restart the flashing of the LEDs
				else
				{
					switch (ledState)
					{
						case RED_ON:
							redLEDOnOff(LED_OFF);
							greenLEDOnOff(LED_OFF);
							ledState = RED_OFF;
							break;
						case RED_OFF:
							redLEDOnOff(LED_ON);
							greenLEDOnOff(LED_OFF);
							ledState = RED_ON;
							break;
						case GREEN_ON:
							greenLEDOnOff(LED_OFF);
							redLEDOnOff(LED_OFF);
							ledState = GREEN_OFF;
							break;
						case GREEN_OFF:
							greenLEDOnOff(LED_ON);
							redLEDOnOff(LED_OFF);
							ledState = GREEN_ON;
							break;
					}
				}
		}
}

/*------------------------------------------------------------
 *  Thread t_button
 *      Poll the button
 *      Signal if button pressed
 *------------------------------------------------------------*/
osThreadId_t t_button;        /* id of thread to poll button */

void buttonThread (void *arg) {
    int state ; // current state of the button
    int bCounter ; // button bounce counter
    state = BUTTONUP ;
	
	while (1) {
        osDelay(10);  // 10 ticks delay - 10ms
        switch (state) {
            case BUTTONUP:
                if (isPressed()) {
                    state = BUTTONDOWN ;
                    osEventFlagsSet(evtFlags, MASK(PRESS_EVT));
                }
                break ;
            case BUTTONDOWN:
                if (!isPressed()) {
                    state = BUTTONBOUNCE ;
                    bCounter = BOUNCE_COUNT ;
                }
                break ;
            case BUTTONBOUNCE:
							  // Decrement bCounter till zero
                if (bCounter > 0) bCounter -- ;
								// if button is pressed, restart button bounce
                if (isPressed()) {
                    state = BUTTONDOWN ; }
								// if button is not pressed and bCounter is finished
                else if (bCounter == 0) {
                    state = BUTTONUP ;
                }
                break ;
        }
    }
}


/*----------------------------------------------------------------------------
 * Application main
 *   Initialise I/O
 *   Initialise kernel
 *   Create threads
 *   Start kernel
 *---------------------------------------------------------------------------*/
 
int main (void) { 
    // System Initialization
    SystemCoreClockUpdate();

    // Initialise peripherals
    configureGPIOoutput();
    configureGPIOinput();
 
    // Initialize CMSIS-RTOS
    osKernelInitialize();
    
    // Create event flags
    evtFlags = osEventFlagsNew(NULL);

    // Create threads
		t_flashLEDs = osThreadNew(flashLEDs, NULL, NULL);
		t_button = osThreadNew(buttonThread, NULL, NULL); 
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
