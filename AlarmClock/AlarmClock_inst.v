module AlarmClock_inst(input clk, reset, input [3:0] switches,buttons, output [31:0] leds);
	AlarmClock u0 (
		.buttons_export  (buttons),  //  buttons.export
		.clk_clk         (clk),         //      clk.clk
		.leds_export     (leds),     //     leds.export
		.reset_reset_n   (reset),   //    reset.reset_n
		.switches_export (switches)  // switches.export
	);
endmodule
