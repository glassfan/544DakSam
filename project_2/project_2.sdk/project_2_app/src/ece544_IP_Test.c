
/**
*
* @file ece544_IP_Test.c
*
* @author Roy Kravitz (roy.kravitz@pdx.edu)
* @modified by Srivatsa Yogendra (srivatsa@pdx.edu)
* @modified by Chetan Bornarkar (bchetan@pdx.edu)
* @modified by Sam Burkhart (skb@pdx.edu)
* @copyright Portland State University, 2014-2015, 2016-2017
*
* This file implements the ECE_544 Spring 2017 Project 1 design.
*
* The project is to control the HSV(24-bit, 8-bit's per channel) color wheel via a rotary encoder module and push-buttons,
* convert the HSV color to a 24-bit RGB value (8-bits per channel) and use those values to drive 2 RGB LED's.
* Display the HSV value in text and as a color patch on an OLED RGB module.
* Read in the RGB LED PWM signals and utilize them to detect the PWM duty cycle in software and hardware.
* Display the RGB duty cycles on the seven segment display.
*
* The requirements of the project are:
*	o Utilize the Rotary Encoder (PMOD_ENC) to control the Hue of an HSV signal
*	o Utilize the Up/Down buttons on the Nexys4DDR to control the Value of an HSV signal
*	o Utilize the Left/Right buttons ton the Nexys4DDR to control the Saturation of an HSV signal
*	o Utilize switch sw0 to toggle software PWM detection (low/off) and hardware PWM detection (high/on)
*	o Utilize LED0 to represent the state of sw0, on when sw0 is high/on for hardware PWM detection and
*		off when sw0 is low/off for software PWM detection
*	o Utilize CPU Reset to perform the system reset
*	o Utilize the Rotary Encoder (PMOD_ENC) push-button to close the application
*	o Utilize the PmodOLEDrgb display to print the Hue/Saturation/Value state on the left
*	o Utilize the PmodOLEDrgb display to display a rectangle of the color represented by the current HSV state on the right
*	o Utilize the RGB LED's (RGB1/RGB2) to output the current HSV color after converting the value to RGB.
*	o Utilize the Seven Segment Display to output the PWM DutyCycle for each channel of the RGB signal used to drive the RGB LED's.
*		Digits [7:6] displays the Red RGB duty cycle
*		Digit [5] is left blank
*		Digits [4:3] displays the Green RGB duty cycle
*		Digit [2] is left blank
*		Digits [1:0] displays the Blue RGB duty cycle
*
* Extended features include:
*   o Utilize the Center button on the Nexys4DDR to set the current HSV to Hue = 0, Saturation = 255, and Value = 255
*   o Utilize sw1 to control the behavior of the Saturation/Value buttons:
*   	low/off = Saturation/Value wrap around from 255->0
*   		for value
*   			if at 0 and down button pushed, next value = 255
*   			if at 255 and up button pushed, next value = 0
*   		for saturation
*   			if at 0 and left button pushed, next value = 255
*   			if at 255 and right button pushed, next value = 0
*   	high/on = Saturation/Value stop at ends 0 and 255
*   		for value
*   			if at 0 and down button pushed, next value = 0
*   			if at 255 and up button pushed, next value = 255
*   		for saturation
*   			if at 0 and left button pushed, next value = 0
*   			if at 255 and right button pushed, next value = 255
*   o Utilize sw2 to control the Hue granularity:
*   	low/off = 1-click of the Rotary Encoder changes the Hue by 13 (~255/20 since the Rotary encoder has 20 clicks per 360 degrees or 18 degrees per click)
*   	high/on = 1-click of the Rotary Encoder changes the Hue by 1
*   o Utilize sw3 to control the direction of the Rotary Encoder:
*   	low/off = Increase Hue to the right, Decrease Hue to the left
*   	high/on = Decrease Hue to the right, Increase Hue to the left
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a	rhk	12/22/14	First release of test program.  Builds on the nx4io_test program.
* 2.00  sy	08/22/16	Modified the code and implemented the functionality of color wheel implementation
* 						and detection of the generated PWM for the RGB LED's
* 3.00  skb 04/21/17	Removed test functions and rewrote Test4 to implement ECE544 Project 1
* </pre>
*
* @note
* The minimal hardware configuration for this test is a Microblaze-based system with at least 32KB of memory,
* an instance of Nexys4IO, an instance of the PMod544IOR2,  and an instance of the Xilinx
* UARTLite (used for xil_printf() console output)
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "platform.h"
#include "xparameters.h"
#include "xstatus.h"
#include "nexys4IO.h"
#include "pmodENC.h"
#include "xgpio.h"
#include "xintc.h"
#include "xtmrctr.h"
#include "PmodOLEDrgb.h"


/************************** Constant Definitions ****************************/
// Clock frequencies
#define CPU_CLOCK_FREQ_HZ		XPAR_CPU_CORE_CLOCK_FREQ_HZ
#define AXI_CLOCK_FREQ_HZ		XPAR_CPU_M_AXI_DP_FREQ_HZ

// AXI timer parameters
#define AXI_TIMER_DEVICE_ID		XPAR_AXI_TIMER_0_DEVICE_ID
#define AXI_TIMER_BASEADDR		XPAR_AXI_TIMER_0_BASEADDR
#define AXI_TIMER_HIGHADDR		XPAR_AXI_TIMER_0_HIGHADDR
#define TmrCtrNumber			0

// Definitions for peripheral NEXYS4IO
#define NX4IO_DEVICE_ID		XPAR_NEXYS4IO_0_DEVICE_ID
#define NX4IO_BASEADDR		XPAR_NEXYS4IO_0_S00_AXI_BASEADDR
#define NX4IO_HIGHADDR		XPAR_NEXYS4IO_0_S00_AXI_HIGHADDR

// Definitions for peripheral PMODOLEDRGB
#define RGBDSPLY_DEVICE_ID		XPAR_PMODOLEDRGB_0_DEVICE_ID
#define RGBDSPLY_GPIO_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_BASEADDR
#define RGBDSPLY_GPIO_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_HIGHADD
#define RGBDSPLY_SPI_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_BASEADDR
#define RGBDSPLY_SPI_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_HIGHADDR

// Display Constants
#define HUE_ROW		1
#define SAT_ROW		3
#define VAL_ROW		5
#define PRINT_COL	4

// Definitions for peripheral PMODENC
#define PMODENC_DEVICE_ID		XPAR_PMODENC_0_DEVICE_ID
#define PMODENC_BASEADDR		XPAR_PMODENC_0_S00_AXI_BASEADDR
#define PMODENC_HIGHADDR		XPAR_PMODENC_0_S00_AXI_HIGHADDR

// Constants related to the Rotary Encoder
// There are:
//		- 20 clicks per rotation
//      - 18 counts per click match 360 degree's per rotation
//		- 13 counts per click match a count to 260 per rotation (0-247) which can be stored in 8 bits

#define CLICKS_PER_ROTATION		20
#define COUNT_PER_CLICK			13

// Fixed Interval timer - 100 MHz input clock, 40KHz output clock
// FIT_COUNT_1MSEC = FIT_CLOCK_FREQ_HZ * .001
#define FIT_IN_CLOCK_FREQ_HZ	CPU_CLOCK_FREQ_HZ
#define FIT_CLOCK_FREQ_HZ		40000
#define FIT_COUNT				(FIT_IN_CLOCK_FREQ_HZ / FIT_CLOCK_FREQ_HZ)
#define FIT_COUNT_1MSEC			40

#define MAX_PWM_COUNT			1000000

// GPIO parameters
#define GPIO_0_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_0_INPUT_0_CHANNEL		1
#define GPIO_0_OUTPUT_0_CHANNEL		2

#define GPIO_1_DEVICE_ID			XPAR_AXI_GPIO_1_DEVICE_ID
#define GPIO_1_INPUT_0_CHANNEL		1
#define GPIO_1_OUTPUT_0_CHANNEL		2

// Interrupt Controller parameters
#define INTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define FIT_INTERRUPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_FIT_TIMER_0_INTERRUPT_INTR


/************************** Variable Definitions ****************************/
unsigned long timeStamp = 0;


/************************** Function Prototypes *****************************/
void usleep(u32 usecs);

void PMDIO_itoa(int32_t value, char *string, int32_t radix);
void PMDIO_puthex(PmodOLEDrgb* InstancePtr, uint32_t num);
void PMDIO_putnum(PmodOLEDrgb* InstancePtr, int32_t num, int32_t radix);
void RunProject1(void);
void FIT_Handler(void);
int AXI_Timer_initialize(void);
uint32_t HSV_to_RGB(uint8_t hue, uint8_t sat, uint8_t val);
void bin2bcd(unsigned long bin, unsigned char *bcd);
int do_init();

PmodENC 	pmodENC_inst;				// PmodENC instance ref
PmodOLEDrgb	pmodOLEDrgb_inst;			// PmodOLED instance ref
XGpio		GPIOInst0;					// GPIO instance
XGpio		GPIOInst1;					// 2nd GPIO instance
XIntc 		IntrptCtlrInst;				// Interrupt Controller instance
XTmrCtr		AXITimerInst;				// PWM timer instance

volatile u32	gpio_in;				// GPIO input port

volatile u8 	motor_speed;
volatile u8 	motor_direction;
volatile u32 	motor_control;
volatile u32	gpio_in_1;

u16 RotaryCnt;

// 8-bit values for the previous and current HSV signals
// Previous values are used determine changes and update
// the displays/signals accordingly
u8	Hue;
u8	PrevHue = 255;

u8	Sat = 255;
u8	PrevSat = 0;

u8  Val = 255;
u8	PrevVal = 0;

// Flag used to determine if using hardware or software PWM detection is selected.
u8 use_hw_PWM_det;

// For each channel of the RGB signal (Red, Green, Blue)...
// Track the HighCycles for software PWM detection
// Track the DutyCycle and Previous DutyCycle to display
// Track the BCD converted DutyCycle for pushing to the seven segment display
u32 RedTotalCycles = 0;
u32 RedHighCycles = 0;
u32 RedDutyCycle = 0;
u8  RedDutyCycleBCD[10];
u8  RedPWM = 0;
u8  PrevRedPWM = 0;

u32 GreenTotalCycles = 0;
u32 GreenHighCycles = 0;
u32 GreenDutyCycle = 0;
u8  GreenDutyCycleBCD[10];
u8  GreenPWM = 0;
u8  PrevGreenPWM = 0;

u32 BlueTotalCycles = 0;
u32 BlueHighCycles = 0;
u32 BlueDutyCycle = 0;
u8  BlueDutyCycleBCD[10];
u8  BluePWM = 0;
u8  PrevBluePWM = 0;


uint64_t 				timestamp = 0L;			// used in delay msec

/************************** MAIN PROGRAM ************************************/
int main()
{
	int sts;
    init_platform();

    sts = do_init();		// initialize the peripherals
    if (XST_SUCCESS != sts)
    {
    	exit(1);
    }

    microblaze_enable_interrupts();		// enable the interrupts

	xil_printf("ECE 544 Project 1\r\n");
	xil_printf("By Sam Burkhart.  21-April 2017\r\n");

	// Project1 code, implements the Rotary Encoder, Push-Buttons, and switch inputs
	// as well as the PMOD OLED RGB display, RGB LED's, MonoChrome LED's and Seven Segment Displays.
	RunProject1();

	// We will do this in a busy-wait loop
	// pressing the Rotary Encoder Button
	// causes the loop to terminate
	timeStamp = 0;

	// Clear all the display digits and the OLED display at the end of the program
	NX410_SSEG_setAllDigits(SSEGLO, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_end(&pmodOLEDrgb_inst);

    cleanup_platform();
    exit(0);
}


/**
 * Function Name: do_init()
 *
 * Return: XST_FAILURE or XST_SUCCESS
 *
 * Description: Initialize the AXI timer, gpio, interrupt, FIT timer, Encoder,
 * 				OLED display
 */
int do_init()
{
	int status;

	// initialize the Nexys4 driver and (some of)the devices
	status = (uint32_t) NX4IO_initialize(NX4IO_BASEADDR);
	if (status == XST_FAILURE)
	{
		exit(1);
	}

	// initialize the PMod544IO driver and the PmodENC and PmodCLP
	status = pmodENC_initialize(&pmodENC_inst, PMODENC_BASEADDR);
	if (status == XST_FAILURE)
	{
		exit(1);
	}

	// Initialize the AXI Timer
	status = AXI_Timer_initialize();
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	// set all of the display digits to blanks and turn off
	// the decimal points using the "raw" set functions.
	// These registers are formatted according to the spec
	// and should remain unchanged when written to Nexys4IO...
	// something else to check w/ the debugger when we bring the
	// drivers up for the first time
	NX4IO_SSEG_setSSEG_DATA(SSEGHI, 0x0058E30E);
	NX4IO_SSEG_setSSEG_DATA(SSEGLO, 0x00144116);

	// Initialize the OLED display
	OLEDrgb_begin(&pmodOLEDrgb_inst, RGBDSPLY_GPIO_BASEADDR, RGBDSPLY_SPI_BASEADDR);

	// initialize the GPIO instances
	status = XGpio_Initialize(&GPIOInst0, GPIO_0_DEVICE_ID);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	// GPIO0 channel 1 is an 32-bit input port.
	// GPIO0 channel 2 is an 32-bit output port.

	// initialize the 2nd GPIO instance
	status = XGpio_Initialize(&GPIOInst1, GPIO_1_DEVICE_ID);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	// GPIO1 channel 1 is an 32-bit input port.
	// GPIO1 channel 2 is an 32-bit output port.

	XGpio_SetDataDirection(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL, 0xFF);
	XGpio_SetDataDirection(&GPIOInst0, GPIO_0_OUTPUT_0_CHANNEL, 0x00);
	XGpio_SetDataDirection(&GPIOInst1, GPIO_1_INPUT_0_CHANNEL, 0xFF);
	XGpio_SetDataDirection(&GPIOInst1, GPIO_1_OUTPUT_0_CHANNEL, 0x00);

	// initialize the interrupt controller
	status = XIntc_Initialize(&IntrptCtlrInst, INTC_DEVICE_ID);
	if (status != XST_SUCCESS)
	{
	   return XST_FAILURE;
	}

	// connect the fixed interval timer (FIT) handler to the interrupt
	status = XIntc_Connect(&IntrptCtlrInst, FIT_INTERRUPT_ID,
						   (XInterruptHandler)FIT_Handler,
						   (void *)0);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;

	}

	// start the interrupt controller such that interrupts are enabled for
	// all devices that cause interrupts.
	status = XIntc_Start(&IntrptCtlrInst, XIN_REAL_MODE);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	// enable the FIT interrupt
	XIntc_Enable(&IntrptCtlrInst, FIT_INTERRUPT_ID);
	return XST_SUCCESS;
}

/****************************************************************************
*
* AXI timer initializes it to generate out a 4Khz signal, Which is given to the Nexys4IO module as clock input.
* DO NOT MODIFY
*
*****************************************************************************/
int AXI_Timer_initialize(void){

	uint32_t status;				// status from Xilinx Lib calls
	u32		ctlsts;		// control/status register or mask

	status = XTmrCtr_Initialize(&AXITimerInst,AXI_TIMER_DEVICE_ID);
		if (status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	status = XTmrCtr_SelfTest(&AXITimerInst, TmrCtrNumber);
		if (status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	ctlsts = XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_EXT_GENERATE_MASK | XTC_CSR_LOAD_MASK |XTC_CSR_DOWN_COUNT_MASK ;
	XTmrCtr_SetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber,ctlsts);

	//Set the value that is loaded into the timer counter and cause it to be loaded into the timer counter
	XTmrCtr_SetLoadReg(AXI_TIMER_BASEADDR, TmrCtrNumber, 24998);
	XTmrCtr_LoadTimerCounterReg(AXI_TIMER_BASEADDR, TmrCtrNumber);
	ctlsts = XTmrCtr_GetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber);
	ctlsts &= (~XTC_CSR_LOAD_MASK);
	XTmrCtr_SetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber, ctlsts);

	ctlsts = XTmrCtr_GetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber);
	ctlsts |= XTC_CSR_ENABLE_TMR_MASK;
	XTmrCtr_SetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber, ctlsts);

	XTmrCtr_Enable(AXI_TIMER_BASEADDR, TmrCtrNumber);
	return XST_SUCCESS;

}

/****************************************************************************/
/**
* RunProject1 - Implements the Project1 code for inputs/outputs
*
* Performs some basic tests on the PmodENC and PmodCLP.  Includes the following tests
* 	1.	check the rotary encoder by displaying the rotary encoder
* 		count in decimal and hex on the LCD display.  Rotate the knob
* 		to change the values up or down.  The pushbuttons can be used
* 		as follows:
* 			o 	press the rotary encoder pushbutton to exit
* 			o 	press BtnUp to clear the count
* 			o 	press BtnR to change rotary encoder
* 				mode to "stop at zero".  This does not appear
* 				to be reversible - not sure why.
* 			o 	press BTNL to change the increment/decrement
* 				value.  Use sw[3:0] to set the new value
*	6.	display the string "357#&CFsw" on the LCD display.  These values
* 		were chosen to check that the bit order is correct.  The screen will
* 		clear in about 5 seconds.
* 	7.	display "Exiting Test 4" on the LCD.  The screen will clear
* 		in about 5 seconds.
*
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunProject1(void)
{
	xil_printf("Starting Project 1 Application\n\r");
	xil_printf("Turn PmodENC shaft to Increase/Decrease Hue\n\r");
	xil_printf("BTNU - Increase Saturation, BTND - Decrease Saturation");
	xil_printf("BNTR - Increase Value, BTNL - Decrease Value\n\r");
	xil_printf("BNTC - Reset Hue = 0, Saturation = 255, Value = 255\n\r");
	xil_printf("SW[0] - High/On = Hardware PWM Detection, Low/Off = Software PWM Detection");
	xil_printf("SW[1] - High/On = Saturation/Value wrap around 0/255, Low/Off = Saturation/Value min=0, max=255 (aka no wrapping)");
	xil_printf("SW[2] - High/On = Rotary Encoder 1-click = Hue change of 1 (negative allowed), Low/Off = Rotary Encoder 1-click = Hue change of 13 (no negative)");
	xil_printf("SW[3] - High/On = Rotary Encoder right decreases/left increases hue, Low/Off = Rotary Encoder right increases/left decreases hue");
	xil_printf("Press Rotary encoder shaft to exit\n\r");

	// values for switches
	u32 led_values;
	u16 sw;
	u16 PrevSw;

	// test the rotary encoder functions
	int RotaryIncr = 1;

	// Initialize the rotary encoder
	// clear the counter of the encoder if initialized to garbage value on power on
	pmodENC_init(&pmodENC_inst, RotaryIncr, true);
	pmodENC_clear_count(&pmodENC_inst);


	// Set up the display output
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst,OLEDrgb_BuildRGB(200, 12, 44));

	// Output 0's to the seven segment display to begin with
	NX4IO_SSEG_putU32Dec(0, true);

	// Start main execution loop
	while(1)
	{
		// check if the rotary encoder pushbutton is pressed
		// exit the loop.
		if ( pmodENC_is_button_pressed(&pmodENC_inst) )
		{
			break;
		}

		// check the center button, and reset the rotary encoder count (Hue) and Saturation/Value counts if pressed.
		if (NX4IO_isPressed(BTNC))      {
			pmodENC_clear_count(&pmodENC_inst);
		}

		// get switch values
		sw = NX4IO_getSwitches();

		// Set the HW/SW PWM Detection status to LED0
		led_values = NX4IO_getLEDS_DATA();

		// mask LED values, only write first 4 bits for Project 1(really only bit 0, but others could be used for debug)
		led_values = led_values & 0xF;

		// write LED values
		NX4IO_setLEDs(led_values);

		// read the new value from the rotary encoder and show it on the display
		pmodENC_read_count(&pmodENC_inst, &RotaryCnt);

		// Convert the RGB Duty Cycles to BCD for display on the seven-segment display
		bin2bcd(RedDutyCycle, RedDutyCycleBCD);
		bin2bcd(BlueDutyCycle, BlueDutyCycleBCD);
		bin2bcd(GreenDutyCycle, GreenDutyCycleBCD);

		// Display the RGB Duty Cycles on the seven segment display
		NX4IO_SSEG_setDigit(SSEGHI, DIGIT7, RedDutyCycleBCD[8]);
		NX4IO_SSEG_setDigit(SSEGHI, DIGIT6, RedDutyCycleBCD[9]);

		NX4IO_SSEG_setDigit(SSEGHI, DIGIT4, GreenDutyCycleBCD[8]);
		NX4IO_SSEG_setDigit(SSEGLO, DIGIT3, GreenDutyCycleBCD[9]);

		NX4IO_SSEG_setDigit(SSEGLO, DIGIT1, BlueDutyCycleBCD[8]);
		NX4IO_SSEG_setDigit(SSEGLO, DIGIT0, BlueDutyCycleBCD[9]);

		PrevSw = sw;
	} // rotary button has been pressed - exit the loop
	xil_printf("\n\rProject 1 Application complete\n\r");

	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	usleep(05000000);

	return;
}


/*********************** HELPER FUNCTIONS ***********************************/

/****************************************************************************/
/**
* insert delay (in microseconds) between instructions.
*
* This function should be in libc but it seems to be missing.  This emulation implements
* a delay loop with (really) approximate timing; not perfect but it gets the job done.
*
* @param	usec is the requested delay in microseconds
*
* @return	*NONE*
*
* @note
* This emulation assumes that the microblaze is running @ 100MHz and takes 15 clocks
* per iteration - this is probably totally bogus but it's a start.
*
*****************************************************************************/

static const u32	DELAY_1US_CONSTANT	= 15;	// constant for 1 microsecond delay

void usleep(u32 usec)
{
	volatile u32 i, j;

	for (i = 0; i < usec; i++)
	{
		for (j = 0; j < DELAY_1US_CONSTANT; j++);
	}
	return;
}

/*********************** DISPLAY-RELATED FUNCTIONS ***********************************/

/****************************************************************************/
/**
* Converts an integer to ASCII characters
*
* algorithm borrowed from ReactOS system libraries
*
* Converts an integer to ASCII in the specified base.  Assumes string[] is
* long enough to hold the result plus the terminating null
*
* @param 	value is the integer to convert
* @param 	*string is a pointer to a buffer large enough to hold the converted number plus
*  			the terminating null
* @param	radix is the base to use in conversion,
*
* @return  *NONE*
*
* @note
* No size check is done on the return string size.  Make sure you leave room
* for the full string plus the terminating null in string
*****************************************************************************/
void PMDIO_itoa(int32_t value, char *string, int32_t radix)
{
	char tmp[33];
	char *tp = tmp;
	int32_t i;
	uint32_t v;
	int32_t  sign;
	char *sp;

	if (radix > 36 || radix <= 1)
	{
		return;
	}

	sign = ((10 == radix) && (value < 0));
	if (sign)
	{
		v = -value;
	}
	else
	{
		v = (uint32_t) value;
	}

  	while (v || tp == tmp)
  	{
		i = v % radix;
		v = v / radix;
		if (i < 10)
		{
			*tp++ = i+'0';
		}
		else
		{
			*tp++ = i + 'a' - 10;
		}
	}
	sp = string;

	if (sign)
		*sp++ = '-';

	while (tp > tmp)
		*sp++ = *--tp;
	*sp = 0;

  	return;
}


/****************************************************************************/
/**
* Write a 32-bit unsigned hex number to PmodOLEDrgb in Hex
*
* Writes  32-bit unsigned number to the pmodOLEDrgb display starting at the current
* cursor position.
*
* @param num is the number to display as a hex value
*
* @return  *NONE*
*
* @note
* No size checking is done to make sure the string will fit into a single line,
* or the entire display, for that matter.  Watch your string sizes.
*****************************************************************************/
void PMDIO_puthex(PmodOLEDrgb* InstancePtr, uint32_t num)
{
  char  buf[9];
  int32_t   cnt;
  char  *ptr;
  int32_t  digit;

  ptr = buf;
  for (cnt = 7; cnt >= 0; cnt--) {
    digit = (num >> (cnt * 4)) & 0xF;

    if (digit <= 9)
	{
      *ptr++ = (char) ('0' + digit);
	}
    else
	{
      *ptr++ = (char) ('a' - 10 + digit);
	}
  }

  *ptr = (char) 0;
  OLEDrgb_PutString(InstancePtr,buf);

  return;
}


/****************************************************************************/
/**
* Write a 32-bit number in Radix "radix" to LCD display
*
* Writes a 32-bit number to the LCD display starting at the current
* cursor position. "radix" is the base to output the number in.
*
* @param num is the number to display
*
* @param radix is the radix to display number in
*
* @return *NONE*
*
* @note
* No size checking is done to make sure the string will fit into a single line,
* or the entire display, for that matter.  Watch your string sizes.
*****************************************************************************/
void PMDIO_putnum(PmodOLEDrgb* InstancePtr, int32_t num, int32_t radix)
{
  char  buf[16];

  PMDIO_itoa(num, buf, radix);
  OLEDrgb_PutString(InstancePtr,buf);

  return;
}


/**************************** INTERRUPT HANDLERS ******************************/

/****************************************************************************/
/**
* Fixed interval timer interrupt handler
*
* Reads the GPIO port which reads back the hardware generated PWM wave for the RGB Leds
* and the duty cycles for the harware PWM detection module
*
* Updates the DutyCycle if the change flag is set, signifying that the HSV value changed
*
* Update the
*
* @note
*
 *****************************************************************************/

void FIT_Handler(void)
{
	// Read the GPIO port to read back the generated PWM signal for RGB led's in the lower 3 bits
	// The Upper 24-bits are used to read the duty-cycle of the hardware PWM detector logic.
	//   - gpio_in[31:24] are the Red Duty Cycle
	//   - gpio_in[23:16] are the Green Duty Cycle
	//   - gpio_in[15:8] are the Blue Duty Cycle
	gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);

	// Read the 2nd GPIO port inputs for the SA/SB motor sensors (Hall Effect Sensor output)
	gpio_in_1 = XGpio_DiscreteRead(&GPIOInst1, GPIO_1_INPUT_0_CHANNEL);


	// motor_speed = 128;	 // 255 is 100%, 0 is 0% duty cycle
	// motor_direction = 0;  // motor direction: 0 = forward, 1 = backward
	// motor_control = (motor_direction << 8) | motor_speed;
	//Set motor control output to 2, leaving the direction bit 0 and turning on the enable pin
	motor_control = 0x2;
	XGpio_DiscreteWrite(&GPIOInst1, GPIO_1_OUTPUT_0_CHANNEL, motor_control);
	xil_printf("Writing Motor Direction and Speed\n\r");
	xil_printf("GPIO_IN_1: %x\n\r", gpio_in_1);

}

/* ------------------------------------------------------------ */
/***	HSV_to_RGB
**
**	Parameters:
**		hue		- Hue of color
**		sat		- Saturation of color
**		val		- Value of color
**
**	Return Value:
**		RGB representation of input color in 24-bit (888) color format
**
**	Errors:
**		none
**
**	Description:
**		Converts an HSV value into a 888 RGB color used to drive the RGB LED's
**		Based on the OLEDrgb_BuildHSV() function from the PmodOLEDrgb.c file
**
**
*/
uint32_t HSV_to_RGB(uint8_t hue, uint8_t sat, uint8_t val){
   uint8_t region, remain, p, q, t;
   uint8_t R,G,B;
   region = hue/43;
   remain = (hue - (region * 43))*6;
   p = (val * (255-sat))>>8;
   q = (val * (255 - ((sat * remain)>>8)))>>8;
   t = (val * (255 - ((sat * (255 - remain))>>8)))>>8;

   switch(region){
      case 0:
       R = val;
       G = t;
       B = p;
       break;
      case 1:
       R = q;
       G = val;
       B = p;
       break;
      case 2:
       R = p;
       G = val;
       B = t;
       break;
       case 3:
       R = p;
       G = q;
       B = val;
       break;
       case 4:
       R = t;
       G = p;
       B = val;
       break;
       default:
       R = val;
       G = p;
       B = q;
       break;
   }
   return (((u32)R<<16) | ((u32)G<<8) | (u32)B);
}
