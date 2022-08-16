module stack (
	clk,
	reset,
	q,
	d,
	push,
	pop,
);

	parameter WIDTH = 32;
	parameter DEPTH = 32;

	input                    clk;
	input                    reset;
	input      [WIDTH - 1:0] d;
	output reg [WIDTH - 1:0] q;
	input                    push;
	input                    pop;

	reg [DEPTH - 1:0] ptr;
	reg [WIDTH - 1:0] stack [0:(1 << DEPTH) - 1];

	always @(posedge clk) begin
		if (reset)
			ptr <= 0;
		else if (push)
			ptr <= ptr + 1;
		else if (pop)
			q <= stack[ptr];
	end

	always @(posedge clk) begin
		if (reset)
			ptr <= 0;
		else if (push)
			stack[ptr] <= d;
		else if (pop)
			ptr <= ptr - 1;
	end

endmodule