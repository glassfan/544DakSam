`timescale 1ns / 1ps

// n4fpga.v - Top level module for the ECE 544 Project 1
//
// Copyright Sam Burkhart, Portland State University, 2017
// 
// Created By:	Roy Kravitz
// Modified By: Sam Burkhart
// Date:		21-April-2017
// Version:		1.1
//
// Description:
// ------------
// This module provides the top level for the Getting Started hardware.
// The module assume that a PmodOLED is plugged into the JA 
// expansion ports and that a PmodENC is plugged into the JD expansion 
// port (bottom row).
//
//  The module has been expanded from the Getting Started tutorial
//  to include a wider GPIO port (32-bit instead of 8-bit) to 
//  accomodate 3x8-bit PWM Duty Cycle values from the PWM_DET module output.
//  The PWM_DET module reads in the duty cycle of the RGB PWM signals for RGB1
//  and performs hardware PWM detection.  The code was also modified to 
//  output the RGB PWM signals to the GPIO ports first 3 bit positions
//////////////////////////////////////////////////////////////////////
module n4fpga(
    input				clk,			// 100Mhz clock input
    input				btnC,			// center pushbutton
    input				btnU,			// UP (North) pusbhbutton
    input				btnL,			// LEFT (West) pushbutton
    input				btnD,			// DOWN (South) pushbutton  - used for system reset
    input				btnR,			// RIGHT (East) pushbutton
	input				btnCpuReset,	// CPU reset pushbutton
    input	[15:0]		sw,				// slide switches on Nexys 4
    output	[15:0] 		led,			// LEDs on Nexys 4   
    output              RGB1_Blue,      // RGB1 LED (LD16) 
    output              RGB1_Green,
    output              RGB1_Red,
    output              RGB2_Blue,      // RGB2 LED (LD17)
    output              RGB2_Green,
    output              RGB2_Red,
    output [7:0]        an,             // Seven Segment display
    output [6:0]        seg,
    output              dp,             // decimal point display on the seven segment 
    
    input				uart_rtl_rxd,	// USB UART Rx and Tx on Nexys 4
    output				uart_rtl_txd,	
    
	inout   [7:0]       JA,             // JA PmodOLED connector 
	                                    // both rows are used 
    output	[7:0] 		JB,				// JB Pmod connector 
                                        // Unused. Can be used for debuggin purposes 
    output	[7:0] 		JC,				// JC Pmod connector - debug signals
										// Can be used for debug purposes
	input	[7:0]		JD				// JD Pmod connector - PmodENC signals
);

// internal variables
// Clock and Reset 
wire				sysclk;             // 
wire				sysreset_n, sysreset;

// Rotary encoder 
wire				rotary_a, rotary_b, rotary_press, rotary_sw;

// GPIO pins, expanded to 32-bits each
wire	[31:0]	    gpio_in;				// embsys GPIO input port
wire	[31:0]	    gpio_out;				// embsys GPIO output port
wire    [31:0]      gpio_1_in;              // embsys GPIO 1 input port
wire    [31:0]      gpio_1_out;             // embsys GPIO 1 output port

// Internal Clock for PWM_DET
wire                clk_10MHz;

// OLED pins 
wire 				pmodoledrgb_out_pin1_i, pmodoledrgb_out_pin1_io, pmodoledrgb_out_pin1_o, pmodoledrgb_out_pin1_t; 
wire 				pmodoledrgb_out_pin2_i, pmodoledrgb_out_pin2_io, pmodoledrgb_out_pin2_o, pmodoledrgb_out_pin2_t; 
wire 				pmodoledrgb_out_pin3_i, pmodoledrgb_out_pin3_io, pmodoledrgb_out_pin3_o, pmodoledrgb_out_pin3_t; 
wire 				pmodoledrgb_out_pin4_i, pmodoledrgb_out_pin4_io, pmodoledrgb_out_pin4_o, pmodoledrgb_out_pin4_t; 
wire 				pmodoledrgb_out_pin7_i, pmodoledrgb_out_pin7_io, pmodoledrgb_out_pin7_o, pmodoledrgb_out_pin7_t; 
wire 				pmodoledrgb_out_pin8_i, pmodoledrgb_out_pin8_io, pmodoledrgb_out_pin8_o, pmodoledrgb_out_pin8_t; 
wire 				pmodoledrgb_out_pin9_i, pmodoledrgb_out_pin9_io, pmodoledrgb_out_pin9_o, pmodoledrgb_out_pin9_t; 
wire 				pmodoledrgb_out_pin10_i, pmodoledrgb_out_pin10_io, pmodoledrgb_out_pin10_o, pmodoledrgb_out_pin10_t;

// RGB LED 
wire                w_RGB1_Red, w_RGB1_Blue, w_RGB1_Green;

// RGB Duty Cycles, driven by the PWM_DET module
wire    [7:0]       w_Red_DC;
wire    [7:0]       w_Green_DC;
wire    [7:0]       w_Blue_DC;

// LED pins 
wire    [15:0]      led_int;                // Nexys4IO drives these outputs

// make the connections to the GPIO port.  Most of the bits are unused in the Getting
// Started project but GPIO's provide a convenient way to get the inputs and
// outputs from logic you create to and from the Microblaze.  For example,
// you may decide that using an axi_gpio peripheral is a good way to interface
// your hardware pulse-width detect logic with the Microblaze.  Our application
// is simple.
// Wrap the RGB led output back to the application program for software pulse-width detect
// Expanded to 32-bits:
//    8-bits for the red duty cycle from the PWM_DET hardware PWM detection module
//    8-bits for the green duty cycle from the PWM_DET hardware PWM detection module
//    8-bits for the blue duty cycle from the PWM_DET hardware PWM detection module
//    5 empty bits, set to 0
//    3-bits for the red, gree, and blue RGB1 PWM signals, used for software PWM detection
assign gpio_in = {w_Red_DC, w_Green_DC, w_Blue_DC, 5'b0, w_RGB1_Red, w_RGB1_Green, w_RGB1_Blue};

assign w_RGB1_Red = RGB1_Red;
assign w_RGB1_Blue = RGB1_Blue;
assign w_RGB1_Green = RGB1_Green;

// Drive the leds from the signal generated by the microblaze 
assign led = led_int;                   // LEDs are driven by led

// make the connections
// system-wide signals
assign sysclk = clk;
assign sysreset_n = btnCpuReset;		// The CPU reset pushbutton is asserted low.  The other pushbuttons are asserted high
										// but the microblaze for Nexys 4 expects reset to be asserted low
assign sysreset = ~sysreset_n;			// Generate a reset signal that is asserted high for any logic blocks expecting it.

// Pmod OLED connections 
assign JA[0] = pmodoledrgb_out_pin1_io;
assign JA[1] = pmodoledrgb_out_pin2_io;
assign JA[2] = pmodoledrgb_out_pin3_io;
assign JA[3] = pmodoledrgb_out_pin4_io;
assign JA[4] = pmodoledrgb_out_pin7_io;
assign JA[5] = pmodoledrgb_out_pin8_io;
assign JA[6] = pmodoledrgb_out_pin9_io;
assign JA[7] = pmodoledrgb_out_pin10_io;

// JB Connector connections can be used for debug purposes
// assign JB = 8'b0000000;
assign JB[0] = gpio_1_out[0];   // Direction for HB3
assign JB[1] = gpio_1_out[1];   // Enable for HB3, PWM signal
// assign gpio_1_in[0] = JB[2];    // SA, Sensor A input
// assign gpio_1_in[1] = JB[3];    // SB, Sensor B input
assign gpio_1_in[0] = gpio_1_out[0];    // SA, Sensor A input
assign gpio_1_in[1] = gpio_1_out[1];    // SB, Sensor B input

// JC Connector pins can be used for debug purposes 
assign JC = 8'h00; 

// PmodENC signals
// JD - bottom row only
// Pins are assigned such that turning the knob to the right
// causes the rotary count to increment.
assign rotary_a = JD[5];
assign rotary_b = JD[4];
assign rotary_press = JD[6];
assign rotary_sw = JD[7];

//MOTOR_CONTROLLER motor_controller(
//    .clk(clk_10MHz),
//    .reset_n(sysreset_n),
//    .motor_speed_in(gpio_1_out[7:0]),
//    .motor_direction_in(gpio_1_out[8]),
//    .motor_direction_out(JB[0]),
//    .motor_speed_out(JB[1])
//);

// instantiate the PWM hardware module
// inputs: Red, Green, and Blue PWM signals from the RGB1 LED inputs, and a 10MHz clock
// outputs: Red, Green, and Blue PWM Duty Cycle calculated in hardware
PWM_DET pwm_det(
	.clk(clk_10MHz),
    .Red(RGB1_Red),
    .Green(RGB1_Green),
    .Blue(RGB1_Blue),
    .RedDC_out(w_Red_DC),
    .GreenDC_out(w_Green_DC),
    .BlueDC_out(w_Blue_DC)
    );
    
// instantiate the embedded system
embsys EMBSYS
       (// PMOD OLED pins 
        .PmodOLEDrgb_out_pin10_i(pmodoledrgb_out_pin10_i),
	    .PmodOLEDrgb_out_pin10_o(pmodoledrgb_out_pin10_o),
	    .PmodOLEDrgb_out_pin10_t(pmodoledrgb_out_pin10_t),
	    .PmodOLEDrgb_out_pin1_i(pmodoledrgb_out_pin1_i),
	    .PmodOLEDrgb_out_pin1_o(pmodoledrgb_out_pin1_o),
	    .PmodOLEDrgb_out_pin1_t(pmodoledrgb_out_pin1_t),
	    .PmodOLEDrgb_out_pin2_i(pmodoledrgb_out_pin2_i),
	    .PmodOLEDrgb_out_pin2_o(pmodoledrgb_out_pin2_o),
	    .PmodOLEDrgb_out_pin2_t(pmodoledrgb_out_pin2_t),
	    .PmodOLEDrgb_out_pin3_i(pmodoledrgb_out_pin3_i),
	    .PmodOLEDrgb_out_pin3_o(pmodoledrgb_out_pin3_o),
	    .PmodOLEDrgb_out_pin3_t(pmodoledrgb_out_pin3_t),
	    .PmodOLEDrgb_out_pin4_i(pmodoledrgb_out_pin4_i),
	    .PmodOLEDrgb_out_pin4_o(pmodoledrgb_out_pin4_o),
	    .PmodOLEDrgb_out_pin4_t(pmodoledrgb_out_pin4_t),
	    .PmodOLEDrgb_out_pin7_i(pmodoledrgb_out_pin7_i),
	    .PmodOLEDrgb_out_pin7_o(pmodoledrgb_out_pin7_o),
	    .PmodOLEDrgb_out_pin7_t(pmodoledrgb_out_pin7_t),
	    .PmodOLEDrgb_out_pin8_i(pmodoledrgb_out_pin8_i),
	    .PmodOLEDrgb_out_pin8_o(pmodoledrgb_out_pin8_o),
	    .PmodOLEDrgb_out_pin8_t(pmodoledrgb_out_pin8_t),
	    .PmodOLEDrgb_out_pin9_i(pmodoledrgb_out_pin9_i),
	    .PmodOLEDrgb_out_pin9_o(pmodoledrgb_out_pin9_o),
	    .PmodOLEDrgb_out_pin9_t(pmodoledrgb_out_pin9_t),
	    // GPIO pins 
        .GPIO_tri_i(gpio_in),
        .GPIO2_tri_o(gpio_out),
        .GPIO3_tri_i(gpio_1_in),
        .GPIO4_tri_o(gpio_1_out),
        // Pmod Rotary Encoder
	    .pmodENC_A(rotary_a),
        .pmodENC_B(rotary_b),
        .pmodENC_btn(rotary_press),
        .pmodENC_sw(rotary_sw),
        // RGB1/2 Led's 
        .RGB1_Blue(RGB1_Blue),
        .RGB1_Green(RGB1_Green),
        .RGB1_Red(RGB1_Red),
        .RGB2_Blue(RGB2_Blue),
        .RGB2_Green(RGB2_Green),
        .RGB2_Red(RGB2_Red),
        // Seven Segment Display anode control  
        .an(an),
        .dp(dp),
        .led(led_int),
        .seg(seg),
        // Push buttons and switches  
        .btnC(btnC),
        .btnD(btnD),
        .btnL(btnL),
        .btnR(btnR),
        .btnU(btnU),
        .clk_10MHz(clk_10MHz),
        .sw(sw),
        // reset and clock 
        .sysreset_n(sysreset_n),
        .sysclk(sysclk),
        // UART pins 
        .uart_rtl_rxd(uart_rtl_rxd),
        .uart_rtl_txd(uart_rtl_txd));
        
// Tristate buffers for the pmodOLEDrgb pins
// generated by PMOD bridge component.  Many
// of these signals are not tri-state.
IOBUF pmodoledrgb_out_pin1_iobuf
(
    .I(pmodoledrgb_out_pin1_o),
    .IO(pmodoledrgb_out_pin1_io),
    .O(pmodoledrgb_out_pin1_i),
    .T(pmodoledrgb_out_pin1_t)
);

IOBUF pmodoledrgb_out_pin2_iobuf
(
    .I(pmodoledrgb_out_pin2_o),
    .IO(pmodoledrgb_out_pin2_io),
    .O(pmodoledrgb_out_pin2_i),
    .T(pmodoledrgb_out_pin2_t)
);

IOBUF pmodoledrgb_out_pin3_iobuf
(
    .I(pmodoledrgb_out_pin3_o),
    .IO(pmodoledrgb_out_pin3_io),
    .O(pmodoledrgb_out_pin3_i),
    .T(pmodoledrgb_out_pin3_t)
);

IOBUF pmodoledrgb_out_pin4_iobuf
(
    .I(pmodoledrgb_out_pin4_o),
    .IO(pmodoledrgb_out_pin4_io),
    .O(pmodoledrgb_out_pin4_i),
    .T(pmodoledrgb_out_pin4_t)
);

IOBUF pmodoledrgb_out_pin7_iobuf
(
    .I(pmodoledrgb_out_pin7_o),
    .IO(pmodoledrgb_out_pin7_io),
    .O(pmodoledrgb_out_pin7_i),
    .T(pmodoledrgb_out_pin7_t)
);

IOBUF pmodoledrgb_out_pin8_iobuf
(
    .I(pmodoledrgb_out_pin8_o),
    .IO(pmodoledrgb_out_pin8_io),
    .O(pmodoledrgb_out_pin8_i),
    .T(pmodoledrgb_out_pin8_t)
);

IOBUF pmodoledrgb_out_pin9_iobuf
(
    .I(pmodoledrgb_out_pin9_o),
    .IO(pmodoledrgb_out_pin9_io),
    .O(pmodoledrgb_out_pin9_i),
    .T(pmodoledrgb_out_pin9_t)
);

IOBUF pmodoledrgb_out_pin10_iobuf
(
    .I(pmodoledrgb_out_pin10_o),
    .IO(pmodoledrgb_out_pin10_io),
    .O(pmodoledrgb_out_pin10_i),
    .T(pmodoledrgb_out_pin10_t)
);


endmodule

