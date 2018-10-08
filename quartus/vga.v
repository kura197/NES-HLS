module VGA(
    input PCK,
    input RST,
    output [3:0] vga_r,
    output [3:0] vga_g,
    output [3:0] vga_b, 
    output vga_hs,
    output vga_vs,
    output [15:0] VramAddr,
    input [5:0] VramData
);

localparam slide = 80;

wire [9:0] hcnt,vcnt;
wire hdispan = (slide <= hcnt && hcnt < 10'd512 + slide);
wire vdispan = (vcnt < 10'd480);
wire VGAspan = hdispan & vdispan;

HVGEN hvgen(PCK,RST,vga_hs,vga_vs,hcnt,vcnt);

CONVRGB convrgb(VramData, VGAspan, vga_r, vga_g, vga_b);

wire [8:0] hdoublecnt = hcnt[9:1];
wire [8:0] vdoublecnt = vcnt[9:1];

//(256, 240) -> (512, 480)
assign VramAddr = (vdoublecnt << 8) + hdoublecnt - (slide >> 1);

endmodule

