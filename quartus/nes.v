module NES(
    input clk,
    input clk2x,
    input pck,  //25MHz
    input resetn,
    input nes_reset,
    input [15:0] nmi_vec,
    input [15:0] res_vec,
    input [15:0] irq_vec,
    input [7:0] key,
    output [3:0] VGA_R,
    output [3:0] VGA_G,
    output [3:0] VGA_B, 
    output VGA_HS,
    output VGA_VS,
    output [15:0] PC
);

wire [15:0] avmm_1_rw_address;
wire avmm_1_rw_byteenable;
wire avmm_1_rw_read;      
wire [7:0] avmm_1_rw_readdata;
wire avmm_1_rw_write;     
wire [7:0] avmm_1_rw_writedata;

wire [15:0] VramAddr;
wire [5:0] VramData;

reg [2:0] sync_resetn;
always @(posedge clk or negedge resetn) begin
    if (!resetn) begin
        sync_resetn <= 3'b0;
    end else begin
        sync_resetn <= {sync_resetn[1:0], 1'b1};
    end
end

exec_nes nes (
  // Interface: clock (clock end)
  .clock               ( clk ), // 1-bit clk input
  // Interface: reset (reset end)
  .resetn              ( sync_resetn[2] ), // 1-bit reset_n input
  // Interface: clock2x (clock end)
  .clock2x             ( clk2x ), // 1-bit clk input
  // Interface: call (conduit sink)
  .start               ( 1'b1 ), // 1-bit valid input
  .busy                ( ), // 1-bit stall output
  // Interface: return (conduit source)
  .done                ( ), // 1-bit valid output
  .stall               ( 1'b0 ), // 1-bit stall input
  // Interface: returndata (conduit source)
  .returndata          ( PC ), // 16-bit data output
  // Interface: VRAM (conduit sink)
  .VRAM                ( 64'h0 ), // 64-bit data input
  // Interface: nmi_vec (conduit sink)
  .nmi_vec             ( nmi_vec ), // 16-bit data input
  // Interface: res_vec (conduit sink)
  .res_vec             ( res_vec ), // 16-bit data input
  // Interface: irq_vec (conduit sink)
  .irq_vec             ( irq_vec ), // 16-bit data input
  // Interface: key (conduit sink)
  .key                 ( key ), // 8-bit data input
  // Interface: res (conduit sink)
  .res                 ( nes_reset ), // 1-bit data input
  // Interface: avmm_1_rw (avalon start)
  .avmm_1_rw_address   ( avmm_1_rw_address    ), // 16-bit address output
  .avmm_1_rw_byteenable( avmm_1_rw_byteenable ), // 1-bit byteenable output
  .avmm_1_rw_read      ( avmm_1_rw_read       ), // 1-bit read output
  .avmm_1_rw_readdata  ( avmm_1_rw_readdata   ), // 8-bit readdata input
  .avmm_1_rw_write     ( avmm_1_rw_write      ), // 1-bit write output
  .avmm_1_rw_writedata ( avmm_1_rw_writedata  )  // 8-bit writedata output
);

VRAM vram (
    .clock ( clk ),
    .data ( avmm_1_rw_writedata ),
    .rdaddress ( VramAddr ),
    .wraddress ( avmm_1_rw_address ),
    .wren ( avmm_1_rw_write ),
    .q ( VramData )
);

VGA vga(
    .PCK(pck),
    .RST(nes_reset),
    .vga_r(VGA_R),
    .vga_g(VGA_G),
    .vga_b(VGA_B), 
    .vga_hs(VGA_HS),
    .vga_vs(VGA_VS),
    .VramAddr(VramAddr),
    .VramData(VramData)
);


endmodule
