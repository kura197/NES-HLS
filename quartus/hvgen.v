module HVGEN(
    input pck,rst,
    output reg vga_hs,vga_vs,
    output reg [9:0]    hcnt,vcnt
);



parameter HMAX = 800;
wire hcntend = (hcnt == HMAX - 10'h001);

always @(posedge pck)
    if(rst)
        hcnt <= 10'h000;
    else
        if(hcntend)
            hcnt <= 10'h000;
        else
            hcnt <= hcnt + 1'b1;

parameter VMAX = 525;

always @(posedge pck)
    if(rst)
        vcnt <= 10'h000;
    else if(hcntend) begin
        if(vcnt == VMAX - 10'h001)	
            vcnt <= 10'h000;
        else
            vcnt <= vcnt + 1'b1;
    end

//parameter HSSTART = 663;
//parameter HSEND = 759;
//parameter VSSTART = 449;
//parameter VSEND = 451;
parameter HSSTART = 655;
parameter HSEND = 751;
parameter VSSTART = 489;
parameter VSEND = 491;

always @(posedge pck)
    if(rst)
        vga_hs <= 1'b1;
    else if(hcnt == HSSTART)
        vga_hs = 1'b0;
    else if(hcnt == HSEND)
        vga_hs = 1'b1;

always @(posedge pck)
    if(rst)
        vga_vs <= 1'b1;
    else if(hcnt == HSSTART)    begin
            if(vcnt == VSSTART)
                vga_vs = 1'b0;
            else if(vcnt == VSEND)
                vga_vs = 1'b1;
    end

endmodule
