library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
port ( 
	RESET : IN std_logic;
	SMCLK : IN std_logic;
	ROW   : OUT std_logic_vector(0 to 7);
	LED   : OUT std_logic_vector(7 downto 0)
);
end ledc8x8;

architecture main of ledc8x8 is
    signal cnt : std_logic_vector(20 downto 0) := (others => '0');
    signal state : std_logic_vector(1 downto 0) := "00";
    signal leds : std_logic_vector(7 downto 0) := (others => '0');
    signal rows : std_logic_vector(7 downto 0) := (others => '0');
    signal ce   : std_logic;

begin

	generator: process(SMCLK, RESET)
    begin
		if RESET = '1' then 
			cnt <= "000000000000000000000";
      elsif rising_edge(smclk) then 
            cnt <= cnt + 1;
            if (cnt(7 downto 0) = "11111111") then
					ce <= '1';
				else
					ce <= '1';
					end if;
				if cnt = "111000010000000000000" then
            	state <= state + 1;
				end if;
      end if;
    end process generator;
   

    rotation: process(RESET, ce, SMCLK)
	begin	
		if RESET = '1' then
			rows <= "10000000"; 
		elsif SMCLK'event and SMCLK = '1' and ce = '1' then
			rows <= rows(0) & rows(7 downto 1); 
		end if;
	end process rotation;

	--------------------------



    dekoder: process(rows)
	begin
		if(state = "00") then
			case rows is
				when "10000000" => leds <= "10011001";
				when "01000000" => leds <= "10011001";
				when "00100000" => leds <= "10011001";
				when "00010000" => leds <= "10011001";
				when "00001000" => leds <= "10011001";
				when "00000100" => leds <= "10011001";
				when "00000010" => leds <= "11000011";
				when "00000001" => leds <= "11100111";
				when others =>     leds <= "11111111";
			end case;
		elsif(state = "01") then
			case rows is
				when "10000000" => leds <= "11111111";
				when "01000000" => leds <= "11111111";
				when "00100000" => leds <= "11111111";
				when "00010000" => leds <= "11111111";
				when "00001000" => leds <= "11111111";
				when "00000100" => leds <= "11111111";
				when "00000010" => leds <= "11111111";
				when "00000001" => leds <= "11111111";
				when others =>     leds <= "11111111";
			end case;
		elsif(state = "10") then
			case rows is
				when "10000000" => leds <= "00011000";
				when "01000000" => leds <= "00000000";
				when "00100000" => leds <= "00000000";
				when "00010000" => leds <= "00100100";
				when "00001000" => leds <= "00111100";
				when "00000100" => leds <= "00111100";
				when "00000010" => leds <= "00111100";
				when "00000001" => leds <= "00111100";
				when others =>     leds <= "11111111";
			end case;
		else
			case rows is
				when "10000000" => leds <= "11111111";
				when "01000000" => leds <= "11111111";
				when "00100000" => leds <= "11111111";
				when "00010000" => leds <= "11111111";
				when "00001000" => leds <= "11111111";
				when "00000100" => leds <= "11111111";
				when "00000010" => leds <= "11111111";
				when "00000001" => leds <= "11111111";
				when others =>     leds <= "11111111";
			end case;
		end if;
	end process dekoder;
	

	ROW <= rows;
	LED <= leds;



end main;
