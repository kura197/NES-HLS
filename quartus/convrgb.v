//R,G,B
module CONVRGB(
	input [5:0] color,
    input VGAspan,
    output [3:0] vga_r,
    output [3:0] vga_g,
    output [3:0] vga_b
);

reg [11:0] getrgb;
assign vga_r = VGAspan ? getrgb[11:8] : 4'h0;
assign vga_g = VGAspan ? getrgb[7:4] : 4'h0;
assign vga_b = VGAspan ? getrgb[3:0] : 4'h0;

always @*
    case(color)
        6'h00: getrgb = {4'h7,4'h7,4'h7};
        6'h01: getrgb = {4'h2,4'h2,4'h8};
        6'h02: getrgb = {4'h0,4'h0,4'ha};
        6'h03: getrgb = {4'h4,4'h0,4'ha};
        6'h04: getrgb = {4'h8,4'h0,4'h7};
        6'h05: getrgb = {4'ha,4'h0,4'h1};
        6'h06: getrgb = {4'ha,4'h0,4'h0};
        6'h07: getrgb = {4'h7,4'h1,4'h0};
        6'h08: getrgb = {4'h4,4'h3,4'h0};
        6'h09: getrgb = {4'h0,4'h4,4'h0};
        6'h0a: getrgb = {4'h0,4'h5,4'h0};
        6'h0b: getrgb = {4'h0,4'h4,4'h1};
        6'h10: getrgb = {4'hb,4'hb,4'hb};
        6'h11: getrgb = {4'h0,4'h7,4'he};
        6'h12: getrgb = {4'h2,4'h3,4'he};
        6'h13: getrgb = {4'h8,4'h0,4'hf};
        6'h14: getrgb = {4'hb,4'h0,4'hb};
        6'h15: getrgb = {4'he,4'h0,4'h5};
        6'h16: getrgb = {4'hd,4'h3,4'h0};
        6'h17: getrgb = {4'hc,4'h5,4'h1};
        6'h18: getrgb = {4'h8,4'h7,4'h0};
        6'h19: getrgb = {4'h0,4'h9,4'h0};
        6'h1a: getrgb = {4'h0,4'ha,4'h0};
        6'h1b: getrgb = {4'h0,4'h9,4'h4};
        6'h20: getrgb = {4'hf,4'hf,4'hf};
        6'h21: getrgb = {4'h4,4'hb,4'hf};
        6'h22: getrgb = {4'h6,4'h7,4'hf};
        6'h23: getrgb = {4'ha,4'h8,4'hf};
        6'h24: getrgb = {4'hf,4'h7,4'hf};
        6'h25: getrgb = {4'hf,4'h7,4'hb};
        6'h26: getrgb = {4'hf,4'h7,4'h6};
        6'h27: getrgb = {4'hf,4'h9,4'h3};
        6'h28: getrgb = {4'hf,4'hb,4'h3};
        6'h29: getrgb = {4'h8,4'hd,4'h1};
        6'h2a: getrgb = {4'h4,4'hd,4'h4};
        6'h2b: getrgb = {4'h5,4'hf,4'h9};
        6'h30: getrgb = {4'hf,4'hf,4'hf};
        6'h31: getrgb = {4'ha,4'he,4'hf};
        6'h32: getrgb = {4'hc,4'hd,4'hf};
        6'h33: getrgb = {4'hd,4'hc,4'hf};
        6'h34: getrgb = {4'hf,4'hc,4'hf};
        6'h35: getrgb = {4'hf,4'hc,4'hd};
        6'h36: getrgb = {4'hf,4'hb,4'hb};
        6'h37: getrgb = {4'hf,4'hd,4'ha};
        6'h38: getrgb = {4'hf,4'he,4'ha};
        6'h39: getrgb = {4'he,4'hf,4'ha};
        6'h3a: getrgb = {4'ha,4'hf,4'hb};
        6'h3b: getrgb = {4'hb,4'hf,4'hc};
        6'h0c: getrgb = {4'h1,4'h3,4'h5};
        6'h0d: getrgb = {4'h0,4'h0,4'h0};
        6'h0e: getrgb = {4'h0,4'h0,4'h0};
        6'h0f: getrgb = {4'h0,4'h0,4'h0};
        6'h1c: getrgb = {4'h0,4'h8,4'h8};
        6'h1d: getrgb = {4'h0,4'h0,4'h0};
        6'h1e: getrgb = {4'h0,4'h0,4'h0};
        6'h1f: getrgb = {4'h0,4'h0,4'h0};
        6'h2c: getrgb = {4'h0,4'he,4'hd};
        6'h2d: getrgb = {4'h7,4'h7,4'h7};
        6'h2e: getrgb = {4'h0,4'h0,4'h0};
        6'h2f: getrgb = {4'h0,4'h0,4'h0};
        6'h3c: getrgb = {4'h9,4'hf,4'hf};
        6'h3d: getrgb = {4'hb,4'hb,4'hb};
        6'h3e: getrgb = {4'h0,4'h0,4'h0};
        6'h3f: getrgb = {4'h0,4'h0,4'h0};
        default:getrgb = {4'h0,4'h0,4'h0};
    endcase

endmodule
