module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// hsv
	hsv_h,
	hsv_s,
	hsv_v,

	//SPI
	sck,
	ssel,
	mosi,
	miso,
	led,
	
	// conduit
	mode,

	//Median filter
	out_p0, out_p1, out_p2, out_p3, out_p4, out_p5, out_p6, out_p7, out_p8,
	median,

	// area 
	red_area, green_area, blue_area, pink_area, orange_area
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;


// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// hsv 
output reg [8:0] 			  hsv_h;//  0 - 360
output reg [7:0] 			  hsv_s;// 0- 255
output reg [7:0] 			  hsv_v; // 0- 255

// SPI
input	sck;
input	ssel;
output reg [31:0]	mosi;
output miso;
output led;


// conduit export
input                         mode;

// Median filter
output reg [31:0] out_p0, out_p1, out_p2, out_p3, out_p4, out_p5, out_p6, out_p7, out_p8;
output reg median;

// area
output reg [11:0] red_area, green_area, blue_area, pink_area, orange_area;

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h00ff00;

wire [7:0]   red, green, blue, grey;
wire [7:0]   red_out, green_out, blue_out;
wire [24:0] hsv_colour;
wire	colour_detect_s, colour_detect_v, colour_detect_blue_s, colour_detect_pink_s, colour_detect_blue_v;
wire colour_detect_orange_v, colour_detect_orange_s;
assign hsv_colour = {hsv_h, hsv_s, hsv_v};

wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

// Detect saturation and vue levels
assign colour_detect_s = (hsv_s > 9'd100) ? 1'b1 : 1'b0;
assign colour_detect_v = (hsv_v > 9'd80) ? 1'b1 : 1'b0;
assign colour_detect_blue_s = (hsv_s > 9'd50) ? 1'b1 : 1'b0;
assign colour_detect_pink_s = (hsv_s > 9'd60) ? 1'b1 : 1'b0;
assign colour_detect_blue_v = (hsv_v > 9'd50) ? 1'b1 : 1'b0;
assign colour_detect_orange_v = (hsv_v > 9'd120) ? 1'b1 : 1'b0;
//assign colour_detect_orange_s = (hsv_s > 9'd150) ? 1'b1 : 1'b0;

// Detect red areas
wire red_detect, red_detect_h, red_detect_hsv;
assign red_detect = red[7] & ~green[7] & ~blue[7];
assign red_detect_h = (hsv_h < 9'd20 || hsv_h > 340) ? 1'b1 : 1'b0;
assign red_detect_hsv = (red_detect_h && colour_detect_s && colour_detect_v) ? 1'b1 : 1'b0;

// Detect blue areas
wire blue_detect, blue_detect_h, blue_detect_hsv;
assign blue_detect = ~red[7] & ~green[7] & blue[7];
assign blue_detect_h = (hsv_h > 9'd130 && hsv_h < 9'd240) ? 1'b1 : 1'b0;
assign blue_detect_hsv = (blue_detect_h && colour_detect_blue_s && colour_detect_blue_v) ? 1'b1 : 1'b0;

// Detect green areas
wire green_detect, green_detect_h, green_detect_hsv;
assign green_detect = ~red[7] & green[7] & ~blue[7];
assign green_detect_h = (hsv_h > 9'd90 && hsv_h < 9'd130) ? 1'b1 : 1'b0;
assign green_detect_hsv = (green_detect_h && colour_detect_s && colour_detect_v) ? 1'b1 : 1'b0;

// Detect orange areas
wire orange_detect, orange_detect_h, orange_detect_hsv;
assign orange_detect = red[7] & green[6] & blue[5];
assign orange_detect_h = (hsv_h > 9'd40 && hsv_h < 9'd90) ? 1'b1 : 1'b0;
assign orange_detect_hsv = (orange_detect_h && colour_detect_s && colour_detect_orange_v) ? 1'b1 : 1'b0;

// Detect pink areas
wire pink_detect, pink_detect_h, pink_detect_hsv;
assign pink_detect = red[7] & green[6] & blue[6];
assign pink_detect_h = (hsv_h > 9'd300) ? 1'b1 : 1'b0;
assign pink_detect_hsv = (pink_detect_h && colour_detect_pink_s && colour_detect_v) ? 1'b1 : 1'b0;

// Find boundary of cursor box

// Highlight detected areas
wire [23:0] red_high_hsv, blue_high_hsv, green_high_hsv, orange_high_hsv, pink_high_hsv;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
//assign red_high  =  red_detect ? {8'hff, 8'h0, 8'h0} : {grey, grey, grey};
//assign blue_high  =  blue_detect ? {8'h0, 8'h0, 8'hff} : {grey, grey, grey};
//assign green_high = green_detect ? {8'h0, 8'hff, 8'h0} : {grey, grey, grey};

assign red_high_hsv  =  red_detect_hsv ? {8'hff, 8'h0, 8'h0} : {grey, grey, grey};
assign blue_high_hsv  =  blue_detect_hsv ? {8'h0, 8'h0, 8'hff} : {grey, grey, grey};
assign green_high_hsv = green_detect_hsv ? {8'h0, 8'hff, 8'h0} : {grey, grey, grey};
assign orange_high_hsv = orange_detect_hsv ? {8'hff, 8'h8c, 8'h0} : {grey, grey, grey}; 
assign pink_high_hsv = pink_detect_hsv ? {8'hff, 8'h69, 8'hb4} : {grey, grey, grey};

// Show bounding box
wire [23:0] new_image, new_image_orange, new_image_pink;
//wire [23:0] new_red_image, new_blue_image, new_green_image, new_orange_image;
wire bb_red_active, bb_blue_active, bb_green_active, bb_orange_active, bb_pink_active;

assign bb_red_active = (x == left_red) | (x == right_red) | (y == top_red) | (y == bottom_red);
assign bb_blue_active = (x == left_blue) | (x == right_blue) | (y == top_blue) | (y == bottom_blue);
assign bb_green_active = (x == left_green) | (x == right_green) | (y == top_green) | (y == bottom_green);
assign bb_orange_active = (x == left_orange) | (x == right_orange) | (y == top_orange) | (y == bottom_orange);
assign bb_pink_active = (x == left_pink) | (x == right_pink) | (y == top_pink) | (y == bottom_pink);

assign new_image = bb_red_active || bb_blue_active || bb_green_active || bb_orange_active || bb_pink_active ? bb_col : red_high_hsv | green_high_hsv | blue_high_hsv | orange_high_hsv | pink_high_hsv;
//assign new_image_orange = bb_orange_active ? bb_col : orange_high_hsv;
//assign new_image_pink = bb_pink_active ? bb_col : pink_high_hsv;
// assign assign new_red_image = bb_red_active ? bb_col : red_high;
//assign new_blue_image = bb_blue_active ? bb_col : blue_high;
//assign new_green_image = bb_green_active ? bb_col : green_high;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};

//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end

//Find first and last red, green, blue, orange and pink pixels
reg [10:0] x_red_min, y_red_min, x_red_max, y_red_max;
reg [10:0] x_blue_min, y_blue_min, x_blue_max, y_blue_max;
reg [10:0] x_green_min, y_green_min, x_green_max, y_green_max;
reg [10:0] x_orange_min, y_orange_min, x_orange_max, y_orange_max;
reg [10:0] x_pink_min, y_pink_min, x_pink_max, y_pink_max;


always@(posedge clk) begin
	if (red_detect_hsv & in_valid) begin	//Update bounds when the pixel is red
		if (x < x_red_min) x_red_min <= x;
		if (x > x_red_max) x_red_max <= x;
		if (y < y_red_min) y_red_min <= y;
		y_red_max <= y;
	end
	if (blue_detect_hsv & in_valid) begin
		if (x < x_blue_min) x_blue_min <= x;
		if (x > x_blue_max) x_blue_max <= x;
		if (y < y_blue_min) y_blue_min <= y;
		y_blue_max <= y;
	end
	if (green_detect_hsv & in_valid) begin
		if (x < x_green_min) x_green_min <= x;
		if (x > x_green_max) x_green_max <= x;
		if (y < y_green_min) y_green_min <= y;
		y_green_max <= y;
	end
	if (orange_detect_hsv & in_valid) begin
		if (x < x_orange_min) x_orange_min <= x;
		if (x > x_orange_max) x_orange_max <= x;
		if (y < y_orange_min) y_orange_min <= y;
		y_orange_max <= y;
	end
	if (pink_detect_hsv & in_valid) begin
		if (x < x_pink_min) x_pink_min <= x;
		if (x > x_pink_max) x_pink_max <= x;
		if (y < y_pink_min) y_pink_min <= y;
		y_pink_max <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		x_red_min <= IMAGE_W-11'h1;
		x_red_max <= 0;
		y_red_min <= IMAGE_H-11'h1;
		y_red_max <= 0;

		x_blue_min <= IMAGE_W-11'h1;
		x_blue_max <= 0;
		y_blue_min <= IMAGE_H-11'h1;
		y_blue_max <= 0;

		x_green_min <= IMAGE_W-11'h1;
		x_green_max <= 0;
		y_green_min <= IMAGE_H-11'h1;
		y_green_max <= 0;

		x_orange_min <= IMAGE_W-11'h1;
		x_orange_max <= 0;
		y_orange_min <= IMAGE_H-11'h1;
		y_orange_max <= 0;
		
		x_pink_min <= IMAGE_W-11'h1;
		x_pink_max <= 0;
		y_pink_min <= IMAGE_H-11'h1;
		y_pink_max <= 0;
	end
end

//Process bounding box at the end of the frame.
reg [2:0] msg_state;
reg [10:0] left_red, right_red, top_red, bottom_red;
reg [10:0] left_blue, right_blue, top_blue, bottom_blue;
reg [10:0] left_green, right_green, top_green, bottom_green;
reg [10:0] left_orange, right_orange, top_orange, bottom_orange;
reg [10:0] left_pink, right_pink, top_pink, bottom_pink;

reg [30:0]	msg_counter;

reg [7:0] frame_count;
always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		left_red <= x_red_min;
		right_red <= x_red_max;
		top_red <= y_red_min;
		bottom_red <= y_red_max;

		left_blue <= x_blue_min;
		right_blue <= x_blue_max;
		top_blue <= y_blue_min;
		bottom_blue <= y_blue_max;

		left_green <= x_green_min;
		right_green <= x_green_max;
		top_green <= y_green_min;
		bottom_green <= y_green_max;

		left_orange <= x_orange_min;
		right_orange <= x_orange_max;
		top_orange <= y_orange_min;
		bottom_orange <= y_orange_max;
		
		left_pink <= x_pink_min;
		right_pink <= x_pink_max;
		top_pink <= y_pink_min;
		bottom_orange <= y_pink_max;
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;

		msg_counter <= msg_counter + 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 3'b01;
			frame_count <= MSG_INTERVAL-1;
			msg_counter <= 0;
		end
	end
	
	//Cycle through message writer states once started 
	if (msg_state != 3'b00 && msg_counter == 10'd2147483648) msg_state <= msg_state + 3'b01;
end
	
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;

parameter RED = 8'b100;
parameter GREEN = 8'b010;
parameter BLUE  = 8'b001;
parameter ORANGE = 8'b110;
parameter PINK = 8'b111;

//parameter MIN_VALUES = 4'b0011; DONT NEED
//parameter MAX_VALUES = 4'b1100;

always@(*) begin	//Write words to FIFO as state machine advances
	case(msg_state)
		3'b000: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		end*/
		3'b001: begin
			msg_buf_in = {8'b0, RED, red_area};	
			msg_buf_wr = 1'b1;
		end
		3'b010: begin
			msg_buf_in = {8'b0, GREEN, green_area}; 
			msg_buf_wr = 1'b1;
		end
		3'b011: begin
			msg_buf_in = {8'b0, BLUE, blue_area};	
			msg_buf_wr = 1'b1;
		end
		3'b100: begin
			msg_buf_in = {8'b0, pink_area};	
			msg_buf_wr = 1'b1;
		end
		3'b101: begin
			msg_buf_in = {8'b0, ORANGE, orange_area};	
			msg_buf_wr = 1'b1;
		end
		default: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
	endcase
end

//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);

HSV hsv_inst(
	.clk(clk),
	.rst(reset_n),
	.rgb_r(red),
	.rgb_g(green),
	.rgb_b(blue),
	.hsv_h(hsv_h),
	.hsv_s(hsv_s),
	.hsv_v(hsv_v)
);

AREA area_inst(
	.clk(clk),
	.x_red_min(x_red_min),
	.x_red_max(x_red_max),
	.x_green_min(x_green_min),
	.x_green_max(x_green_max),
	.x_blue_min(x_blue_min),
	.x_blue_max(x_blue_max),
	.x_pink_min(x_pink_min),
	.x_pink_max(x_pink_max),
	.x_orange_min(x_orange_min),
	.x_orange_max(x_orange_max),
	.red_area(red_area),
	.blue_area(blue_area),
	.green_area(green_area),
	.pink_area(pink_area),
	.orange_area(orange_area)
);

/* Not used anymore, changed to UART
SPI_SLAVE slave_inst(
	.clk(clk),
	.SCK(sck),
	.SSEL(ssel),
	.MOSI(mosi),
	.MISO(miso),
	.LED(led),
	.data_in(msg_buf_in)
);
*/


WINDOW_BUFFER window_inst(
	.clk(clk),
	.in_p0(msg_buf_in),
	.in_p3(msg_buf_in),
	.in_p6(msg_buf_in),
	.out_p0(out_p0), .out_p1(out_p1), .out_p2(out_p2), .out_p3(out_p3), 
	.out_p4(out_p4), .out_p5(out_p5), .out_p6(out_p6), .out_p7(out_p7), .out_p8(out_p8) 
);



MEDIAN median_inst(
	.s1(out_p0), .s2(out_p1), .s3(out_p2),
	.s4(out_p3), .s5(out_p4), .s6(out_p5), 
	.s7(out_p6), .s8(out_p7), .s9(out_p8),
	.med(median)
);

/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	[23:0]	bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						
endmodule