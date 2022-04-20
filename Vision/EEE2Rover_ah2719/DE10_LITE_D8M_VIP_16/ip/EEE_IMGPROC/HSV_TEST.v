`timescale 1 ns/10 ps  // time-unit = 1 ns, precision = 10 ps

module HSV_TEST;

reg clk, reset;
reg [7:0] red, green, blue;
wire [8:0] hsv_h;
wire [7:0] hsv_s, hsv_v;

HSV HSV_DUT(
    .clk(clk),
    .rst(reset),
    .rgb_r(red),
    .rgb_g(green),
    .rgb_b(blue),
    .hsv_h(hsv_h),
    .hsv_s(hsv_s),
    .hsv_v(hsv_v)
);

initial begin
    $dumpfile("hsv_results.vcd");
    $dumpvars(0, HSV_TEST);
end

always begin
    clk = 1'b1; 
    #20; // high for 20 * timescale = 20 ns

    clk = 1'b0;
    #20; // low for 20 * timescale = 20 ns
end

always @(posedge clk) begin
    red = 255;
    green = 0;
    blue = 0;
    //#period; // wait for a period

    if(hsv_h != 0 && hsv_s < 200 && hsv_v < 200) begin
        $display("test failed with rgb_r = 255, rgb_g = 0, rgb_b = 0");
        $display("hsv_h = %d, hsv_s = %d, hsv_v = %d", hsv_h, hsv_s, hsv_v);
    end

    red = 200;
    green = 50;
    blue = 20;
    //#period;

    if(hsv_h < 20 && hsv_s < 200 && hsv_v < 200) begin
        $display("test failed with rgb_r = 200, rgb_g = 50, rgb_b = 20");
        $display("hsv_h = %d, hsv_s = %d, hsv_v = %d", hsv_h, hsv_s, hsv_v);
    end
        
    $finish;
end

endmodule