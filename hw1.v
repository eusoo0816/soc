library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity simple_passthrough is
    Port (
        i_clk      : in STD_LOGIC;
        i_rst      : in STD_LOGIC;
        i_btn1  : in STD_LOGIC; -- 新增：按鈕1 - 增加上限
        i_btn2  : in STD_LOGIC; -- 新增：按鈕2 - 減少上限
        i_data     : in STD_LOGIC_VECTOR(7 downto 0); -- 初始上限值
        o_data     : out STD_LOGIC_VECTOR(7 downto 0) 
    );
end simple_passthrough;

architecture Behavioral of simple_passthrough is
    signal count       : STD_LOGIC_VECTOR(24 downto 0) := (others => '0');  
    signal div_clk     : STD_LOGIC;
    signal led_mask    : STD_LOGIC_VECTOR(7 downto 0) := (others => '0');
    signal integer_count : INTEGER := 0;
    signal max_count   : INTEGER := 0;  -- 由 i_data 初始後可被改變
    signal prev_inc    : STD_LOGIC := '0';
    signal prev_dec    : STD_LOGIC := '0';
begin
    -- clock divider
    process (i_clk, i_rst)
    begin
        if i_rst = '0' then
            count <= (others => '0');
        elsif rising_edge(i_clk) then
            count <= count + 1;
        end if;
    end process;
    div_clk <= count(24);  -- 控制點燈速度

    -- 初始化 max_count
    process (i_clk, i_rst)
    begin
        if i_rst = '0' then
            max_count <= CONV_INTEGER(i_data);
        elsif rising_edge(i_clk) then
            -- 偵測按鈕1上升沿
            if i_btn1 = '1' and prev_inc = '0' then
                max_count <= max_count + 1;
            end if;
            -- 偵測按鈕2上升沿
            if i_btn2 = '1' and prev_dec = '0' then
                if max_count > 1 then  -- 避免小於1
                    max_count <= max_count - 1;
                end if;
            end if;

            -- 更新前一拍狀態
            prev_inc <= i_btn1;
            prev_dec <= i_btn2;
        end if;
    end process;

    -- 控制 LED 點亮
    process (div_clk, i_rst)
    begin
        if i_rst = '0' then
            integer_count <= 0;
            led_mask <= (others => '0');
        elsif rising_edge(div_clk) then
            if integer_count < max_count - 1 then
                integer_count <= integer_count + 1;
            else
                integer_count <= 0;
                if led_mask = "11111111" then
                    led_mask <= (others => '0');
                else
                    led_mask <= led_mask(6 downto 0) & '1';
                end if;
            end if;
        end if;
    end process;

    o_data <= led_mask;

end Behavioral;
