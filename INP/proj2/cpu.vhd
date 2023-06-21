-- cpu.vhd: Simple 8-bit CPU (BrainF*ck interpreter)
-- Copyright (C) 2018 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): xmimoc01
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni z pameti (DATA_RDWR='1') / zapis do pameti (DATA_RDWR='0')
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA obsahuje stisknuty znak klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna pokud IN_VLD='1'
   IN_REQ    : out std_logic;                     -- pozadavek na vstup dat z klavesnice
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- pokud OUT_BUSY='1', LCD je zaneprazdnen, nelze zapisovat,  OUT_WE musi byt '0'
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------

architecture behavioral of cpu is
 
     ---------------------------
	 -- Èítaè PC inc.
	 ---------------------------
	 signal pc_data: std_logic_vector(11 downto 0) := (others => '0');
    signal pc_inc: std_logic := '0'; -- inkrementace
    signal pc_dec: std_logic := '0'; -- dekrementace
	 
	 ---------------------------
	 -- Ukazatel do pamìti inc.
	 ---------------------------
    signal ptr_data: std_logic_vector(9 downto 0) := (others => '0');
    signal ptr_inc: std_logic := '0'; -- inkrementace
    signal ptr_dec: std_logic := '0'; -- dekrementace
	 
	 ---------------------------
	 -- Èítaè CNT inc.
	 ---------------------------
    signal cnt_data: std_logic_vector(7 downto 0) := (others => '0');
    signal cnt_inc: std_logic := '0'; -- inkrementace
    signal cnt_dec: std_logic := '0'; -- dekrementace
    signal cnt_c: std_logic := '0'; -- pomocny
	 
	 ---------------------------
	 -- Multiplexor inc.
	 ---------------------------
    signal multiplex: std_logic_vector(1 downto 0) := "00";
    signal temp: std_logic_vector(7 downto 0) := (others => '0');
	 
	 ---------------------------
	 
	 ---------------------------
	 -- Stavy automatu
	 ---------------------------
    type fsm is (
        state_fetch,
		  state_decode,
        
		  state_ptr_inc,
		  state_ptr_dec,
        
		  state_inc_1,
		  state_inc_2,
		  state_dec_1,
		  state_dec_2,
        
		  state_putchar_1,
		  state_putchar_2,
		  state_getchar,
        
		  state_while_start_1,
		  state_while_start_2,
		  state_while_start_3,
		  state_while_start_4,
          state_while_end_1,
		  state_while_end_2,
		  state_while_end_3,
		  state_while_end_4,
		  state_while_end_5,
		  state_while_end_6,
        
		  state_comments_1,
		  state_comments_2,
		  state_comments_3,
        
		  state_chr_1,
		  state_chr_2,
        
		  state_null,
		  state_others
    );
 
    signal prezent_state: fsm;
    signal next_state: fsm;
	 
	 ---------------------------
	 
begin

	 ---------------------------
	 -- Multiplexor
	 ---------------------------
    multiplexor: process(IN_DATA, DATA_RDATA, multiplex, temp)
    begin
        case (multiplex) is
				when "11" => DATA_WDATA <= temp;
				when "10" => DATA_WDATA <= DATA_RDATA - 1;
				when "01" => DATA_WDATA <= DATA_RDATA + 1;
				when "00" => DATA_WDATA <= IN_DATA;
		  
            when others =>
        end case ;
    end process;
	 ---------------------------
	 
	 ---------------------------
	 -- Èítaè PC
	 ---------------------------
    pc_counter: process(CLK, RESET, pc_data, pc_inc, pc_dec)
    begin
        if (RESET = '1') then
            pc_data <= (others => '0');
        elsif (CLK'event and CLK = '1') then
            if pc_inc = '1' then
                pc_data <= pc_data + 1;
            elsif pc_dec = '1' then
                pc_data <= pc_data - 1;
            end if;
        end if;
        CODE_ADDR <= pc_data;
    end process;
	 ---------------------------
	 
	 ---------------------------
	 -- Ukazatel do pamìti PTR
	 ---------------------------
    pointer_counter: process(CLK, RESET, ptr_data, ptr_inc, ptr_dec)
    begin
        if (RESET = '1') then
            ptr_data <= (others => '0');
        elsif (CLK'event and CLK = '1') then
            if ptr_inc = '1' then
                ptr_data <= ptr_data + 1;
            elsif ptr_dec = '1' then
                ptr_data <= ptr_data - 1;
            end if;
        end if;
        DATA_ADDR <= ptr_data;
    end process;
	 ---------------------------
	 
	 ---------------------------
	 -- Èítaè CNT
	 ---------------------------
    cnt_counter: process(CLK, RESET, cnt_inc, cnt_dec)
    begin
        if (RESET = '1') then
            cnt_data <= (others => '0');
        elsif (CLK'event and CLK = '1') then
            if cnt_inc = '1' then
                cnt_data <= cnt_data + 1;
            elsif cnt_dec = '1' then
                cnt_data <= cnt_data - 1;
            elsif cnt_c = '1' then
                cnt_data <= X"01";
            end if;
        end if;
    end process;
	 ---------------------------
	 
    fsm_start: process(CLK, RESET)
    begin
        if (RESET = '1') then
            prezent_state <= state_fetch;
        elsif (CLK'event and CLK = '1') then
            if EN = '1' then
                prezent_state <= next_state;
            end if;
        end if;
    end process;
 
    fsm_next_state: process(CODE_DATA, IN_VLD, OUT_BUSY, DATA_RDATA, cnt_data, prezent_state)
    begin
        
		  -- Inicializace
		  
        DATA_EN      <= '0';
        OUT_WE       <= '0';
        IN_REQ       <= '0';
        DATA_RDWR    <= '0';
		  CODE_EN      <= '1';
		  pc_inc       <= '0';
		  pc_dec       <= '0';
		  ptr_inc      <= '0';
		  ptr_dec      <= '0';
		  cnt_inc      <= '0';
		  cnt_dec      <= '0';
		  cnt_c        <= '0';
		  multiplex    <= "00";  
		  
        ---------------------------
		  
        case prezent_state is
            when state_fetch =>
                CODE_EN <= '1';
                next_state <= state_decode;
 
            when state_decode =>
                case CODE_DATA is
                    when X"3E" => next_state <= state_ptr_inc;         -- >
                    when X"3C" => next_state <= state_ptr_dec;         -- <
                    when X"2B" => next_state <= state_inc_1;           -- +
                    when X"2D" => next_state <= state_dec_1;           -- -
                    when X"5B" => next_state <= state_while_start_1;   -- [
                    when X"5D" => next_state <= state_while_end_1;     -- ]
                    when X"2E" => next_state <= state_putchar_1;       -- .
                    when X"2C" => next_state <= state_getchar;         -- ,
                    when X"23" => next_state <= state_comments_1;      -- #
                    when X"30" => next_state <= state_chr_1;           -- 0
                    when X"31" => next_state <= state_chr_1;           -- 1
                    when X"32" => next_state <= state_chr_1;           -- 2
                    when X"33" => next_state <= state_chr_1;           -- 3
                    when X"34" => next_state <= state_chr_1;           -- 4
                    when X"35" => next_state <= state_chr_1;           -- 5
                    when X"36" => next_state <= state_chr_1;           -- 6
                    when X"37" => next_state <= state_chr_1;           -- 7
                    when X"38" => next_state <= state_chr_1;           -- 8
                    when X"39" => next_state <= state_chr_1;           -- 9
                    when X"41" => next_state <= state_chr_2;           -- A
                    when X"42" => next_state <= state_chr_2;           -- B
                    when X"43" => next_state <= state_chr_2;           -- C
                    when X"44" => next_state <= state_chr_2;           -- D
                    when X"45" => next_state <= state_chr_2;           -- E
                    when X"46" => next_state <= state_chr_2;           -- F
                    when X"00" => next_state <= state_null;            -- NULL
                    when others => next_state <= state_others;         -- Others
                end case;
					 
				---------------------------------
				-- > -- Inkrementace hodnoty PTR
				---------------------------------
            when state_ptr_inc =>
                ptr_inc <= '1';
                pc_inc <= '1';
                next_state <= state_fetch;
					 
				---------------------------------
				-- < -- Dekrementace hodnoty PTR
				---------------------------------
            when state_ptr_dec =>
                ptr_dec <= '1';
                pc_inc <= '1';
                next_state <= state_fetch;
					 
				---------------------------------
				-- + -- Inkrementace hodnoty
				---------------------------------
            when state_inc_1 =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                next_state <= state_inc_2;
				---------------------------------
					 
				---------------------------------
				-- - -- Dekrementace hodnoty
				---------------------------------
            when state_inc_2 =>
                multiplex <= "01";
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                pc_inc <= '1';
                next_state <= state_fetch;
					 
            when state_dec_1 =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                next_state <= state_dec_2;
					 
            when state_dec_2 =>
                multiplex <= "10";
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                pc_inc <= '1';
                next_state <= state_fetch;
				---------------------------------
					 
				---------------------------------
				-- . -- Tisk hodnoty
				---------------------------------
            when state_putchar_1 =>
                if OUT_BUSY = '1' then
                    next_state <= state_putchar_1;
                else
                    DATA_EN <= '1';
                    DATA_RDWR <= '1';
                    next_state <= state_putchar_2;
                end if;
					 
            when state_putchar_2 =>
                OUT_WE <= '1';
                OUT_DATA <= DATA_RDATA;
                pc_inc <= '1';
                next_state <= state_fetch;
				---------------------------------
					 
				---------------------------------
				-- , -- Naètení hodnoty
				---------------------------------
            when state_getchar =>
                IN_REQ <= '1';
                if IN_VLD = '0' then
                    next_state <= state_getchar;
                else
                    multiplex <= "00";
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    pc_inc <= '1';
                    next_state <= state_fetch;
                end if;
				---------------------------------
					 
				---------------------------------
				-- [ -- Zaèátek while
				---------------------------------
            when state_while_start_1 =>
                pc_inc <= '1';
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                next_state <= state_while_start_2;
					 
            when state_while_start_2 =>
                if DATA_RDATA = X"00" then
                    cnt_c <= '1';
                    next_state <= state_while_start_3;
                else
                    next_state <= state_fetch;
                end if; 
					 
            when state_while_start_3 =>
                if cnt_data = X"00" then
                    next_state <= state_fetch;
                else
                    CODE_EN <= '1';
                    next_state <= state_while_start_4;
                end if; 
					 
            when state_while_start_4 =>
                if CODE_DATA = X"5B" then
                    cnt_inc <= '1';
                elsif CODE_DATA = X"5D" then
                    cnt_dec <= '1';
                end if;
                pc_inc <= '1';
                next_state <= state_while_start_3;

				---------------------------------
				-- ] -- Konec while
				---------------------------------
            when state_while_end_1 =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                next_state <= state_while_end_2;
					 
            when state_while_end_2 =>
                if DATA_RDATA = X"00" then
                    pc_inc <= '1';
                    next_state <= state_fetch;
                else
                    next_state <= state_while_end_3;
                end if;
					 
            when state_while_end_3 =>
                cnt_c <= '1';
                pc_dec <= '1';
                next_state <= state_while_end_4;
					 
            when state_while_end_4 =>
                if cnt_data = X"00" then
                    next_state <= state_fetch;
                else
                    CODE_EN <= '1';
                    next_state <= state_while_end_5;
                end if;
					 
            when state_while_end_5 =>
                if CODE_DATA = X"5D" then
                    cnt_inc <= '1';
                elsif CODE_DATA = X"5B" then
                    cnt_dec <= '1';
                end if;
                next_state <= state_while_end_6;
					 
            when state_while_end_6 =>
                if cnt_data = X"00" then
                    pc_inc <= '1';
                else
                    pc_dec <= '1';
                end if;
                next_state <= state_while_end_4;
				---------------------------------
					 
				---------------------------------
				-- # -- Blokový komentáø
				---------------------------------
            when state_comments_1 =>
                pc_inc <= '1';
                next_state <= state_comments_2;
					 
            when state_comments_2 =>
                CODE_EN <= '1';
                next_state <= state_comments_3;
					 
            when state_comments_3 =>
                if CODE_DATA = X"23" then
                    pc_inc <= '1';
                    next_state <= state_fetch;
                else
                    next_state <= state_comments_1;
                end if;
				---------------------------------
				
				
				---------------------------------
				-- 0-9 A-F --
				---------------------------------
            when state_chr_1 =>
                DATA_EN <= '1';
                pc_inc <= '1';
                multiplex <= "11";
                temp <= CODE_DATA(3 downto 0) & X"0";
                next_state <= state_fetch;
					 
            when state_chr_2 =>
                DATA_EN <= '1';
                pc_inc <= '1';
                multiplex <= "11";
                temp <= (CODE_DATA(3 downto 0) + std_logic_vector(conv_unsigned(9, temp'LENGTH))) & "0000";
                next_state <= state_fetch;
				---------------------------------
				

				---------------------------------
				-- NULL -- 
				---------------------------------
            when state_null =>
                next_state <= state_null;
				---------------------------------
				
				---------------------------------
				-- Ostatní --
				---------------------------------
            when state_others =>
                pc_inc <= '1';
                next_state <= state_fetch;
				---------------------------------
 
        end case;
    end process;
end behavioral;