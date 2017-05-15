`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/13/2017 11:39:11 AM
// Design Name: 
// Module Name: MOTOR_SPEED_DETECTOR
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


module MOTOR_SPEED_DETECTOR(
    input clk,
    input reset_n,
    input motor_sensor_input_a,
    input motor_sensor_input_b,
    output [9:0] motor_speed_count
    );
    parameter COUNT = 1000;
    
    reg [9:0] counter;
    reg [9:0] counter_a;
    reg [9:0] counter_b;
    reg [9:0] counter_out_a;
    reg [9:0] counter_out_b;
    
    assign motor_speed_count = counter_out_a;
    // assign motor_speed_count = 10'b11_1111_1111;
    
        
    initial begin
        counter = 0;
        counter_a = 0;
        counter_b = 0;
        counter_out_a = 0;
        counter_out_b = 0;
    end
    
    always @(posedge motor_sensor_input_a or negedge reset_n) begin
        if (!reset_n) begin
            counter_a <= 0;
            // counter_b <= 0;
            counter_out_a <= 0;
//            counter_out_b <= 0;
        end else if (counter >= COUNT) begin
            counter_out_a <= counter_a;
//            counter_out_b <= counter_b;
            counter_a <= 0;
//            counter_b <= 0;
//            counter <= 0;
        end else 
            counter_a <= counter_a + 1;
    end
        // counter_a <= 10'b11_1111_1111;
        
//    always @(posedge motor_sensor_input_b)
//        counter_b <= counter_b + 1;
        
    always @(posedge clk or negedge reset_n) begin
        if (!reset_n)
            counter <= 0;
        else if (counter >= COUNT)
            counter <= 0;
        else
            counter <= counter + 1;
    end
endmodule
