module MUX(
    a,
    b,
    s,
    out1
);

input [7:0]a, b;
input s;

output [7:0]out1;

reg [7:0]out1;

always@(a,b,s) begin
    if(s==1'b1) begin
        out1=a;
    end
    else begin
        out1=b;
    end
end

endmodule