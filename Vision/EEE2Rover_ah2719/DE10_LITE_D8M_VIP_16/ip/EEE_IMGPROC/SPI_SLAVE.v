// https://www.fpga4fun.com/SPI2.html

module SPI_SLAVE(
    clk,
    SCK,
    MOSI,
    MISO,
    SSEL, 
    LED,
	 data_in
);

input   clk;
input   SCK, SSEL, MOSI;
input [31:0]    data_in;

output MISO;

output LED;

////////////////// SYNCHRONISING ///////////////////

// sync SCK to the clock using a 3-bits shift register
reg[2:0] SCKr;
wire SCK_risingedge = (SCKr[2:1]==2'b01);  // now we can detect SCK rising edges
wire SCK_fallingedge = (SCKr[2:1]==2'b10);  // and falling edges

// sync SSEL to clock
reg [2:0] SSELr;
wire SSEL_active = ~SSELr[1];  // SSEL is active low
wire SSEL_startmessage = (SSELr[2:1]==2'b10);  // message starts at falling edge
wire SSEL_endmessage = (SSELr[2:1]==2'b01);  // message stops at rising edge

// sync MOSI to clock
reg [1:0] MOSIr;
wire MOSI_data = MOSIr[1];

always @(posedge clk) begin
    SCKr <= {SCKr[1:0], SCK};
    SSELr <= {SSELr[1:0], SSEL};
    MOSIr <= {MOSIr[0], MOSI};
end

/////////////////// RECEIVING DATA /////////////////

reg[4:0] bitcnt;
reg byte_received;
reg[31:0] byte_data_received;

always @(posedge clk) begin
    if(~SSEL_active) begin
        bitcnt <= 5'b000;
    end
    else if(SCK_risingedge) begin
        bitcnt <= bitcnt + 5'b0001;
        byte_data_received <= {byte_data_received[30:0], MOSI_data};
    end
    byte_received <= SSEL_active && SCK_risingedge && (bitcnt==5'b11111);
end

reg LED;

always @(posedge clk) begin
    if(byte_received) begin
        LED <= byte_data_received[0];
    end
end

////////////////// TRANSMISSION ///////////////////

reg[31:0] byte_data_sent;

reg[31:0] cnt;

always @(posedge clk) begin
    if(SSEL_startmessage) begin
        cnt <= cnt + 8'h1;
    end
end
always@(posedge clk) begin
    if(SSEL_active) begin
        if(SSEL_startmessage) begin
            byte_data_sent <= data_in;
        end
        else if(SCK_fallingedge) begin
            if(bitcnt==3'b000) begin
                byte_data_sent <= 32'h00;
            end
            else begin
                byte_data_sent <= {byte_data_sent[30:0], 1'b0};
            end
        end
    end
end

assign MISO = byte_data_sent[31];

endmodule