`timescale 1 ns/10 ps  // time-unit = 1 ns, precision = 10 ps

module WINDOW_BUFFER_TEST;

reg clk;
reg rst;
reg valid;
reg [23:0] p0, p3, p6;
output wire [23:0] q0, q1, q2, q3, q4, q5, q6, q7, q8;

WINDOW_BUFFER WINDOW_DUT(
	.clk(clk),
    .reset_n(rst),
    .in_valid(valid),
	.in_p0(p0),
	.in_p3(p3),
	.in_p6(p6),
	.out_p0(q0), .out_p1(q1), .out_p2(q2), .out_p3(q3), 
	.out_p4(q4), .out_p5(q5), .out_p6(q6), .out_p7(q7), .out_p8(q8) 
);

always #10 clk = ~clk;

initial begin
    clk = 0;
    rst = 0;
    p0 <= 24'b1;
    p3 <= 24'b1;
    p6 <= 24'b1;
end

initial begin
    $dumpfile("window_results.vcd");
    $dumpvars(0, WINDOW_BUFFER_TEST);
end

initial begin
    rst <= 0;
    #20
    rst <= 1;
    #20
    rst <= 0;
    valid <= 1;
    repeat(8) @(posedge clk) begin
        p0 <= 24'b1;
        p3 <= 24'b1;
        p6 <= 24'b1;
    end
    repeat(8) @(posedge clk) begin
    end

    $finish;
end

initial begin
    $monitor("q0=%0b, q1=%0b, q2=%0b, q3=%0b, q4=%0b, q5=%0b, q6=%0b, q7=%0b, q8=%0b", q0, q1, q2, q3, q4, q5, q6, q7, q8);
end

endmodule