-- CPU16.VHD 8/18/2000 Dr. C. H. Ting
-- simple portable definition of P16 Minimal Instruction Set Computer Core
-- in VHDL
-- CCC notes:
--    ghdl -s -fsynopsys cpu16.vhdl


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_misc.all;
use ieee.std_logic_unsigned.all;

entity cpu16 is
-- set bus width constant to 16
        generic(width: integer := 16);
        port(
-- standard logic signals can be true, false, tri-stated, or unknown.
        clk, clr: in std_logic;
        write, read: out std_logic;
-- address and data busses
        addr: out std_logic_vector(width downto 0);
        data: in std_logic_vector(width downto 0)
        );
end  cpu16;

architecture archcpu16 of cpu16 is
-- slot counts through the instructions packed in a word
        signal slot: integer range 0 to 3;
-- define stack mechanism
        type stack is array(7 downto 0) of std_logic_vector(width downto 0);
-- control signal definitions
         signal n_stack, r_stack: stack;
-- two stack points are used with each stack to do the pre and post incrementing
-- used in stack push and pops, hencr np and np1, np1=np+1
        signal np, np1, rp, rp1: integer range 0 to 7;
-- select signals
        signal t, n, r, a, i, p: std_logic_vector(width downto 0);
        signal t_in, n_in, r_in, a_in, i_in, p_in: std_logic_vector(width downto 0);
        signal reg_out, alu_out: std_logic_vector(width downto 0);
        signal code: std_logic_vector(4 downto 0);
        signal reg_sel, alu_sel: std_logic_vector(1 downto 0);
        signal npush, npop, rpush, rpop, rsel, tsel, tright, tleft,
               ainc, aload, pinc, pload, msel, psel, z,
               iload, reset: std_logic;
-- define opcodes
        constant jmp : std_logic_vector(4 downto 0) :="00000";
        constant ret : std_logic_vector(4 downto 0) :="00001";
        constant jz  : std_logic_vector(4 downto 0) :="00010";
        constant jnc : std_logic_vector(4 downto 0) :="00011";
        constant call: std_logic_vector(4 downto 0) :="00100";
        constant u005: std_logic_vector(4 downto 0) :="00101"; -- unused
        constant u006: std_logic_vector(4 downto 0) :="00110"; -- unused
        constant u007: std_logic_vector(4 downto 0) :="00111"; -- unused
        constant ftch: std_logic_vector(4 downto 0) :="01000";
        constant ldp : std_logic_vector(4 downto 0) :="01001";
        constant lit : std_logic_vector(4 downto 0) :="01010";
        constant lp  : std_logic_vector(4 downto 0) :="01011";
        constant u012: std_logic_vector(4 downto 0) :="01100"; -- unused
        constant stp : std_logic_vector(4 downto 0) :="01101";
        constant u014: std_logic_vector(4 downto 0) :="01110"; -- unused
        constant st  : std_logic_vector(4 downto 0) :="01111";
        constant com : std_logic_vector(4 downto 0) :="10000";
        constant shl : std_logic_vector(4 downto 0) :="10001";
        constant shr : std_logic_vector(4 downto 0) :="10010";
        constant addc: std_logic_vector(4 downto 0) :="10011";
        constant xorr: std_logic_vector(4 downto 0) :="10100";
        constant andd: std_logic_vector(4 downto 0) :="10101";
        constant addd: std_logic_vector(4 downto 0) :="10111";
        constant pop : std_logic_vector(4 downto 0) :="11000";
        constant lda : std_logic_vector(4 downto 0) :="11001";
        constant dup : std_logic_vector(4 downto 0) :="11010";
        constant over: std_logic_vector(4 downto 0) :="11011";
        constant push: std_logic_vector(4 downto 0) :="11100";
        constant sta : std_logic_vector(4 downto 0) :="11101";
        constant nop : std_logic_vector(4 downto 0) :="11110";
        constant drop: std_logic_vector(4 downto 0) :="11111";

-- instruction opcode meanings                           Forth equivalent
-- jmp  unconditional jump with 10 bit on-page argument  (else)
-- ret  subroutine return, pop R to P                    ;
-- jz   jump if T=0                                      (if)
-- jnc  jump if no carry                                 (-if)
-- call subrountine call with 10 bit on-page argument    :
-- ftch fetch contents of memory using R as pointer      R @ R> 1+ >R
-- ldp  fetch using A and increment A                    A @ DUP 1+ A ! @
-- lit  load immeditate following cell (literal)         LIT
-- ld   fetch using A                                    A @ @
-- stp  store using A and increment A                    A @ DUP 1+ A ! !
-- st   store using A                                    A @ !
-- ???  store using R and increment R                    R ! R> 1+ >R
-- com  invert T, one's complement, including carry      -1 XOR
-- shl  shift T left                                     2*
-- shr  shift T right (carry unchanged)                  2/
-- addc conditional non-destructive add of T and N
--      if the least sig bit of T is true                DUP 1 AND IF OVER + THEN
-- xorr exclusive-or T and N                             XOR
-- andd logical AND T and N                              AND
-- addd add T to N                                       +
-- pop  move, pop from T and push to R                   >R
-- lda  move, pop from A and push to T                   A @
-- dup  duplicate T to N                                 DUP
-- over duplicate N as new T                             OVER
-- push move, pop from R and push to T                   R>
-- sta  move, pop from T and push to A                   A !
-- nop  no operation, delay 1 cycle                      NOP
-- drop discard T                                        DROP
--
--     warning there may be errors in this opcode documentation
--     I could not find the R!+ instruction (?)

begin

-- define the first mux in the diagram
        with alu_sel select
        alu_out <= (t xor n) when "01",
                   (t and n) when "10",
                   (t + n) when "11",
                   (not t) when others;

-- define the second mux
        with reg_sel select
        reg_out <= a when "01",
                   r when "10",
                   data when "11",
                   n when others;

-- instruction latch mux
        with slot select
        code <= i(width-1 downto width-5) when 1,
                i(width-6 downto width-10) when 2,
                i(width-11 downto width-15) when 3,
                ftch when others;

        n <= n_stack(np);
        r <= r_stack(rp);

        r_in <= t when rsel='0' else p;
-- combine lower ten bits in argument with upper bits from the program counter
        p_in <= (p(width downto width-5) & i(width-6 downto 0)) when psel='0' else r;
        addr <= p when msel='0' else a;
        t_in <= alu_out when tsel='0' else reg_out;

-- zero flag
        z <= not(t(15) or t(14) or t(13) or t(12)
                or t(11) or t(10) or t(9) or t(8)
                or t(7) or t(6) or t(5) or t(4)
                or t(3) or t(2) or t(1) or t(0));

        decode: process(code,z,t) begin
                alu_sel<="00";
                reg_sel<="00";
                npush<='0';
                npop<='0';
                rpush<='0';
                rpop<='0';
                rsel<='0';
                tsel<='0';
                tright<='0';
                tleft<='0';
                ainc<='0';
                aload<='0';
                pinc<='0';
                pload<='0';
                msel<='0';
                psel<='0';
                write<='0';
                read<='0';
                iload<='0';
                reset<='0';

-- specify each opcode
        case code is
                when ftch => iload<='1';
                        pinc<='1';
                        read<='1';
                when jmp => pload<='1';
                        npush<='1';
                        rpush<='1';
                        reset<='1';
                when ret => pload<='1';
                        rpop<='1';
                        psel<='1';
                        reset<='1';
                when jz => pload<=z;
                        reset<='1';
                when jnc => pload<= not t(width);
                        reset<='1';
                when call => pload<='1';
                        rpush<='1';
                        rsel<='1';
                        reset<='1';
                when ldp => msel<='1';
                        ainc<='1';
                        tright<='1';
                        tleft<='1';
                        tsel<='1';
                        npush<='1';
                        reg_sel<="11";
                        read<='1';
                when lit => pinc<='1';
                        tright<='1';
                        tleft<='1';
                        tsel<='1';
                        npush<='1';
                        reg_sel<="11";
                        read<='1';

               when lp => msel<='1';     -- ld is typo?
  --            when ld => msel<='1';

                        tright<='1';
                        tleft<='1';
                        tsel<='1';
                        npush<='1';
                        reg_sel<="11";
                        read<='1';
                when stp => msel<='1';
                        ainc<='1';
                        tright<='1';
                        tleft<='1';
                        tsel<='1';
                        npop<='1';
                        reg_sel<="00";
                        write<='1';
--                      data <= t;
                when st => msel<='1';
                        tright<='1';
                        tleft<='1';
                        tsel<='1';
                        npop<='1';
                        reg_sel<="00";
                        write<='1';
--                      data <= t;
                when com => tright<='1';
                        tleft<='1';
                        alu_sel<="00";
                when shr => tright<='1';
                        tleft<='0';
                        alu_sel<="00";
                when shl => tright<='0';
                        tleft<='1';
                        alu_sel<="00";
                when addc => if t(0)='1' then
                        tright<='1';
                        tleft<='1';
                        alu_sel<="11";
                        end if;
                when xorr => tright<='1';
                        tleft<='1';
                        alu_sel<="01";
                        npop<='1';
                when andd => tright<='1';
                        tleft<='1';
                        alu_sel<="10";
                        npop<='1';
                when addd => tright<='1';
                        tleft<='1';
                        alu_sel<="11";
                        npop<='1';
                when pop => tright<='1';
                        tleft<='1';
                        tsel<='1';
                        reg_sel<="10";
                        rpop<='1';
                        npush<='1';
                when lda => tright<='1';
                        tleft<='1';
                        tsel<='1';
                        reg_sel<="01";
                        npush<='1';
                when dup => npush<='1';
                when over => tright<='1';
                        tleft<='1';
                        tsel<='1';
                        reg_sel<="00";
                        npush<='1';
                when push => tright<='1';
                        tleft<='1';
                        tsel<='1';
                        rpush<='1';
                        npop<='1';
                when sta => tright<='1';
                        tleft<='1';
                        tsel<='1';
                        aload<='1';
                        npop<='1';
                when drop => tright<='1';
                        tleft<='1';
                        tsel<='1';
                        npop<='1';
                when others => null;
        end case;
        end process decode;

-- specify synchonous processes

        sync: process(clk,clr) begin
                if clr='1' then
                        slot <= 0;
                        i <= (others => '0');
                        np <= 0;
                        np1 <= 1;
                        rp <= 0;
                        rp1 <= 1;
                        t <= (others => '0');
                        p <= (others => '0');
                        a <= (others => '0');
                        for ii in n_stack'range loop
                               n_stack(ii) <= (others => '1');
                               r_stack(ii) <= (others => '1');
                        end loop;
-- rising edge of clock
                elsif (clk'event and clk='1') then
                        if reset='1' then
                                slot <= 0;
                        else    slot <= slot+1;
                        end if;
                        if iload='1' then
                                i <= data;
                        end if;
                        if aload='1' then
                                a <= t;
                        elsif ainc='1' then
                              a <= a+1;
                        end if;
                        if pload='1' then
                                p <= p_in;
                        elsif pinc='1' then
                                p <= p+1;
                        end if;
                        if npush='1' then
                                n_stack(np) <= t;
                                np <= np+1;
                                np1 <= np1+1;
                        elsif npop='1' then
                                np <= np-1;
                                np1 <= np1-1;
                        end if;
                        if rpush='1' then
                                r_stack(rp) <= r_in;
                                rp <= rp+1;
                                rp1 <= rp1+1;
                        elsif rpop='1' then
                                rp <= rp-1;
                                rp1 <= rp1-1;
                        end if;
                        if tright='1' then
                                if tleft='1' then
                                        t <= t_in;
                                else    t <= t(width) & t(width-1) & t(width-1 downto 1);
                                       end if;
                                elsif tleft='1' then
                                        t <= t(width-1 downto 0) & '0';
                                end if;
                        end if;
                end process sync;

        end archcpu16;