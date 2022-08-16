/* 
 * ALU for simple 8-bit CPU.
 * Copyright (C) 2015 John Tzonevrakis.
 * Licensed under the ISC license. For more details, read the COPYING file.
 *
*/

module alu  (input [7:0] a,b, input [3:0] opcode, output reg [7:0] y);
        /* Decode the instruction */
	always @*
        	case (opcode)
                	4'h0 /* OR */:   y <= a | b;
                	4'h1 /* AND */:   y <= a & b;
                	4'h2 /* NOTA */:   y <= ~a;
                	4'h3 /* XOR */:   y <= a ^ b;
                	4'h4 /* ADD */:   y <= a + b;
                	4'h5 /* SUB */:   y <= a - b;
			4'h6 /* RSHIFT1 */: y <= a >> 1;
			4'h7 /* RSHIFTN */: y <= a >> b;
			default: y <= 8'bZ;
        	endcase
endmodule //alu