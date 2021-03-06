module AREA(
    clk,
    x_red_min,
    x_red_max,
    y_red_min,
    y_red_max,

    x_green_min,
    x_green_max,
    y_green_min,
    y_green_max,

    x_blue_min,
    x_blue_max,
    y_blue_min,
    y_blue_max,

    x_pink_min,
    x_pink_max,
    y_pink_min,
    y_pink_max,

    x_orange_min,
    x_orange_max,
    y_orange_min,
    y_orange_max,
	 
	 red_area,
	 green_area,
	 blue_area,
	 orange_area,
	 pink_area
);

input clk;

input [10:0] x_red_min, x_red_max, y_red_min, y_red_max;
input [10:0] x_green_min, x_green_max, y_green_min, y_green_max;
input [10:0] x_blue_min, x_blue_max, y_blue_min, y_blue_max;
input [10:0] x_pink_min, x_pink_max, y_pink_min, y_pink_max;
input [10:0] x_orange_min, x_orange_max, y_orange_min, y_orange_max;

output reg [15:0] red_area, green_area, blue_area, pink_area, orange_area;

reg [10:0] x_red_diff, y_red_diff, x_green_diff, y_green_diff, x_blue_diff, y_blue_diff;
reg [10:0] x_pink_diff, y_pink_diff, x_orange_diff, y_orange_diff;

always @(posedge clk) begin
    x_red_diff <= x_red_max - x_red_min;
    y_red_diff <= y_red_max - y_red_min;

    x_green_diff <= x_green_max - x_green_min;
    y_green_diff <= y_green_max - y_green_min;

    x_blue_diff <= x_blue_max - x_blue_min;
    y_blue_diff <= y_blue_max - y_blue_min;

    x_pink_diff <= x_pink_max - x_pink_min;
    y_pink_diff <= y_pink_max - y_pink_min;

    x_orange_diff <= x_orange_max - x_orange_min;
    y_orange_diff <= y_orange_max - y_orange_min;
end

always @(negedge clk) begin
    red_area <= x_red_diff * y_red_diff;
    green_area <= x_green_diff * y_green_diff;
    blue_area <= x_blue_diff * y_blue_diff;
    pink_area <= x_pink_diff * y_pink_diff;
    orange_area <= x_orange_diff * y_orange_diff;
end

endmodule