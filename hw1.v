library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity simple_passthrough is
    Port (
        i_clk  : in STD_LOGIC;
        i_rst  : in STD_LOGIC;
        i_btn  : in STD_LOGIC;
        i_data : in STD_LOGIC_VECTOR(7 downto 0);
        o_data : out STD_LOGIC_VECTOR(7 downto 0) 
    );
end simple_passthrough;

architecture Behavioral of simple_passthrough is
    signal count : STD_LOGIC_VECTOR(24 downto 0) := (others => '0');  
    signal div_clk: STD_LOGIC;
    signal led_mask : STD_LOGIC_VECTOR(7 downto 0) := (others => '0');  --led
begin
    process (i_clk, i_rst)
    begin
        if i_rst = '0' then
            count <= (others => '0');
        elsif rising_edge(i_clk) then
            count <= count + 1;
        end if;
    end process;
    div_clk <= count(2);
    
    process (i_clk, i_rst)
    begin
        if i_rst = '0' then
            led_mask <= (others => '0');
        elsif rising_edge(i_clk) then
           if i_btn ='1' then
              led_mask <= "00001111";
           else
              led_mask <=i_data ;
           end if;
        end if;
    end process;
    
    process(div_clk, i_rst)
    begin
        if i_rst = '0' then
            o_data <= (others=>'0');
        elsif rising_edge(div_clk) then
            o_data <=  led_mask;
        end if;
    end process;
end Behavioral;
