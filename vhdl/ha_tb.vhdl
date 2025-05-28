library ieee;
use ieee.std_logic_1164.all;

entity ha_tb is
end ha_tb;

architecture test of ha_tb is
    component ha
        port ( 
            a:       in      std_ulogic;
            b:       in      std_ulogic;
            o:       out     std_ulogic;
            c:       out     std_ulogic
        );
    end component;
    signal a, b, c, o : std_ulogic;
begin
    half_addr:  ha port map(a => a, b =>b, o =>o, c =>c);
    process begin
        a <= 'X';
        b <= 'X';
        wait for 1 ns;
        report "[X,X]: 'o' is " & std_ulogic'image(o);
        report "       'c' is " & std_ulogic'image(c);
        
        a <= '0';
        b <= '0';
        wait for 1 ns;
        report "[0,0]: 'o' is " & std_ulogic'image(o);
        report "       'c' is " & std_ulogic'image(c);
        
        a <= '0';
        b <= '1';
        wait for 1 ns;
        report "[0,1]: 'o' is " & std_ulogic'image(o);
        report "       'c' is " & std_ulogic'image(c);
        
        a <= '1';
        b <= '0';
        wait for 1 ns;
        report "[1,0]: 'o' is " & std_ulogic'image(o);
        report "       'c' is " & std_ulogic'image(c);
        
        a <= '1';
        b <= '1';
        wait for 1 ns;
        report "[1,1]: 'o' is " & std_ulogic'image(o);
        report "       'c' is " & std_ulogic'image(c);
        
        report "End of test - ^C to stop";
        wait;
    end process;
end test;
