library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pong_game is
    Port (
        i_clk       : in  std_logic;  -- 系統時脈
        i_rst       : in  std_logic;  -- 異步重置
        i_btnL  : in  std_logic;  -- 左側按鈕
        i_btnR : in  std_logic;  -- 右側按鈕
        o_leds       : out std_logic_vector(7 downto 0)  -- 8 顆 LED，顯示球的位置或比分
    );
end pong_game;

architecture Behavioral of pong_game is
    -------------------------------------------------------------------
    -- 定義 FSM 狀態
    type state_type is (IDLE, SERVE, MOVE, SCORE, GAME_OVER);
    signal state : state_type := IDLE;
    -------------------------------------------------------------------
    -- 除頻器產生 div_clk (控制球移動速率)
    signal div_clk         : std_logic := '0';
    signal clk_div_counter : INTEGER := 0; 
    constant DIVISOR       : INTEGER := 40000000;  -- 可調整除頻值
    -------------------------------------------------------------------
    -- 與球運動有關之全域暫存器
    -- ball_pos 與 direction 僅由 ball_move process 更新
    signal ball_pos   : integer range 0 to 7 := 0;
    signal direction  : integer range -1 to 1 := 0;  -- -1：左移，1：右移
    -------------------------------------------------------------------
    -- 與發球相關，由 FSM process 指定初始球位置與方向，並以 init_ball 通知 ball_move process
    signal serve_pos : integer range 0 to 7 := 0;
    signal serve_dir : integer range -1 to 1 := 0;
    signal init_ball : std_logic := '0';  -- 僅由 FSM process 寫入

    -- handshake: 由 ball_move process 提供初始化完成回報，僅由 ball_move process 寫入與清除
    signal ball_initAck : std_logic := '0';
    -------------------------------------------------------------------
    -- 分數及錯失事件
     signal left_score, right_score : integer range 0 to 9 := 0;
    -- miss_event: 0 = 無, 1 = 左端未擊打, 2 = 右端未擊打
    signal miss_event  : integer range 0 to 2 := 0;
    -- miss_ack 用於讓 FSM process 告知 ball_move process 清除 miss_event
    signal miss_ack    : std_logic := '0';
	
	signal inc_left, inc_right : std_logic := '0';

begin
    -------------------------------------------------------------------
    -- 除頻器 process：以系統 i_clk 分頻產生 div_clk
    div_process:process(i_clk, i_rst)
    begin
        if i_rst = '0' then
            clk_div_counter <= 0;
            div_clk <= '0';
        elsif rising_edge(i_clk) then
            if clk_div_counter = DIVISOR then
                clk_div_counter <= 0;
                div_clk <= not div_clk;
            else
                clk_div_counter <= clk_div_counter + 1;
            end if;
        end if;
    end process;
    -------------------------------------------------------------------
    -- FSM process (使用系統時脈)
    process(i_clk, i_rst)
    begin
        if i_rst = '0' then
            state       <= IDLE;
            inc_left      <= '0';
            inc_right     <= '0';
            init_ball   <= '0';
            miss_ack    <= '0';
        elsif rising_edge(i_clk) then
		    inc_left      <= '0';
            inc_right     <= '0';		
            case state is
                when IDLE =>
                    if i_btnL = '1' then
                        state     <= SERVE;
                        serve_pos <= 0;
                        serve_dir <= 1;
                        init_ball <= '1';  -- 通知 ball_move process 進行初始化
                    elsif i_btnR = '1' then
                        state     <= SERVE;
                        serve_pos <= 7;
                        serve_dir <= -1;
                        init_ball <= '1';
                    end if;

                when SERVE =>
                    -- 等待 ball_move process 完成初始化 (ball_initAck 為 '1')
                    if ball_initAck = '1' then
                        init_ball <= '0';  -- 清除發球通知，由 FSM 告知 ball_move 已完成初始化
                        state <= MOVE;
                    end if;

                when MOVE =>
                    if miss_event = 1 then
                        inc_right <= '1';
                        state <= SCORE;
                        miss_ack <= '1';
                    elsif miss_event = 2 then
                        inc_left  <= '1';
                        state <= SCORE;
                        miss_ack <= '1';
                    end if;

                when SCORE =>
                    if (i_btnL = '1') or (i_btnR = '1') then
                        if (left_score = 9) or (right_score = 9) then
                            state <= GAME_OVER;
                        else
                            state <= SERVE;
                            if i_btnL = '1' then
                                serve_pos <= 0;
                                serve_dir <= 1;
                            else
                                serve_pos <= 7;
                                serve_dir <= -1;
                            end if;
                            init_ball <= '1';
                        end if;
                    end if;

                when GAME_OVER =>
                    null;
                when others =>
                    null;
            end case;

            -- 清除 miss_ack 訊號 (持續一個 i_clk 脈衝)
            if miss_ack = '1' then
                miss_ack <= '0';
            end if;
        end if;
    end process;
    -------------------------------------------------------------------
    -- 球移動 process (使用 div_clk)
    -- 此 process 為唯一驅動 ball_pos 與 direction 的地方，
    -- 且根據目前 FSM 的狀態更新球的位置。
    ball_moving:process(div_clk, i_rst)
    begin
        if i_rst = '0' then
            ball_pos      <= 0;
            direction     <= 0;
            miss_event  <= 0;
            ball_initAck <= '0';
        elsif rising_edge(div_clk) then
            if init_ball = '1' then
                -- 初始化球的位置與方向，僅讀取 FSM 提供的 serve_pos 與 serve_dir
                ball_pos  <= serve_pos;
                direction <= serve_dir;
                miss_event <= 0;
                ball_initAck <= '1';  -- 回報初始化完成
            else
                -- 若 ball_initAck 已回報初始化，清除它
                if ball_initAck = '1' then
                    ball_initAck <= '0';
                end if;
                if state = MOVE then
                    if (ball_pos = 0 and direction = -1) then
                        if i_btnL = '1' then
                            direction <= 1;  -- 成功擊打，反彈
                        else
                            miss_event <= 1; -- 左側未擊打
                        end if;
                    elsif (ball_pos = 7 and direction = 1) then
                        if i_btnR = '1' then
                            direction <= -1; -- 成功擊打，反彈
                        else
                            miss_event <= 2; -- 右側未擊打
                        end if;
                    else
                        ball_pos <= ball_pos + direction;
                    end if;

                    if miss_ack = '1' then
                        miss_event <= 0;
                    end if;
                end if;
            end if;
        end if;
    end process;
	    ------------------------------------------------------------------
    -- 分數計算 process
    ------------------------------------------------------------------
    scoreADD_process: process(i_clk,i_rst)
    begin
        if rising_edge(i_clk) then
            if i_rst = '0' then
                left_score  <= 0;
                right_score <= 0;
            else
                if inc_left = '1' then
                    left_score <= left_score + 1;
                end if;
                if inc_right = '1' then
                    right_score <= right_score + 1;
                end if;
            end if;
        end if;
    end process;
    -------------------------------------------------------------------
    -- LED 顯示 process
    leddispay_process:process(state, ball_pos, right_score, left_score,i_rst)
        variable led_temp : std_logic_vector(7 downto 0);
    begin
        case state is
            when MOVE | SERVE =>
                led_temp := (others => '0');
                
                 if init_ball = '1' then
                -- 當正在初始化時，直接顯示發球位置
                led_temp(serve_pos) := '1';
                 else
                led_temp(ball_pos) := '1';
                end if;
              
                
                o_leds <= led_temp;
            when SCORE | GAME_OVER =>
                o_leds <= std_logic_vector(to_unsigned(left_score, 4)) & std_logic_vector(to_unsigned(right_score, 4));
            when others =>
                o_leds <= (others => '0');
        end case;
    end process;

end Behavioral;
