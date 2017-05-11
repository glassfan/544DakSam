`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/10/2017 03:53:12 PM
// Design Name: 
// Module Name: MOTOR_CONTROLLER
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

// This should be a parameters, it sets the period for the motor control PWD signal
`define PWM_COUNT 255

module MOTOR_CONTROLLER(
    input clk,
    input reset_n,
    input motor_direction_in,
    input [7:0] motor_speed_in,
    output motor_direction_out,
    output motor_speed_out
    );
    
    reg [31:0] counter;
    reg [31:0] pwm_counter;
    
    reg prev_motor_direction;
    reg motor_direction;
    reg motor_speed;
    reg change_direction;

    
    assign motor_speed_out = motor_speed;
    assign motor_direction_out = motor_direction;
    
    initial begin
        counter = 0;
        prev_motor_direction = 0;
        motor_direction = 0;
        motor_speed = 0;
        change_direction = 0;
    end
    
    // setup a free running counter that counts up to PWM_COUNT, the PWM period
    always @(posedge clk)
        if ((counter < `PWM_COUNT) & (reset_n))
            counter <= counter + 1;
        else
            counter <= 0;
         
    always @(posedge clk) begin
        // reset the speed to 0 and set the change_direction flag
        // if (!reset_n)
        //     motor_speed <= 0;
            // change_direction <= 1;
        if ((counter < motor_speed_in) & (reset_n))
            motor_speed <= 1;
        else
            motor_speed <= 0;
        
        // if the change direction flag is set, change the motor direction and clear the flag
        // TODO: should there be a delay to ensure the motor is stopped?
        // if (change_direction) begin
        //     motor_direction <= prev_motor_direction;
        //     change_direction <= 0;
        // end
        
        // if the previous motor direction is different from the current direction,
        // stop the motor and set the change_direction flag
        // if (prev_motor_direction != motor_direction_in) begin
        //     motor_speed <= 0;
        //     change_direction <= 1;
        // end
            
        // prev_motor_direction <= motor_direction_in;
    end
endmodule
