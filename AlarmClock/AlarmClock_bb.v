
module AlarmClock (
	clk_clk,
	inputs_export,
	leds_export,
	reset_reset_n);	

	input		clk_clk;
	input	[7:0]	inputs_export;
	output	[31:0]	leds_export;
	input		reset_reset_n;
endmodule
