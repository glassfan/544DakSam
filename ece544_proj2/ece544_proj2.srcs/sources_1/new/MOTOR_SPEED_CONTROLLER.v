`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/13/2017 10:23:07 AM
// Design Name: 
// Module Name: MOTOR_SPEED_CONTROLLER
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module MOTOR_SPEED_CONTROLLER(
    input clk,
    input reset_n,
    input [7:0] motor_speed_in,
    output motor_enable_out
    );
	
	parameter PWM_COUNT = 255;
    
    reg [31:0] counter;
    reg [31:0] pwm_counter;
    
    reg motor_enable;
    
    assign motor_enable_out = motor_enable;
    
    initial begin
        counter = 0;
        motor_enable = 0;
    end
    
    // setup a free running counter that counts up to PWM_COUNT, the PWM period
    always @(posedge clk)
        if ((counter < PWM_COUNT) & (reset_n))
            counter <= counter + 1;
        else
            counter <= 0;
         
    // while the requested motor speed is larger than the counter, output a 1 to the motor enable
    always @(posedge clk) begin
        if ((counter < motor_speed_in) & (reset_n))
            motor_enable <= 1;
        else
            motor_enable <= 0;
    end
endmodule
