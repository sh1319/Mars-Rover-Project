module MEDIAN_SORT_2(
    a,
    b,
    min,
    max
);

input [7:0]a, b;

output [7:0]min, max;

wire cmp;

MEDIAN_COMPARE c1(a,b,cmp);
MUX m1(a,b,cmp,min);
MUX m2(b,a,cmp,max);

endmodule