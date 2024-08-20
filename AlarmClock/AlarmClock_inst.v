module AlarmClock_inst(input clk, reset, input [7:0] inputs, output [31:0] leds);
	AlarmClock u0 (
		.clk_clk       (clk),       //    clk.clk
		.inputs_export (inputs), // inputs.export
		.leds_export   (leds),   //   leds.export
		.reset_reset_n (reset)  //  reset.reset_n
	);

endmodule 