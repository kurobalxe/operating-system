#!/bin/bash
# 算命大师.sh - 输入出生日期，输出属相和星座

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 属相（生肖）数组
ZODIAC_ANIMALS=("鼠" "牛" "虎" "兔" "龙" "蛇" "马" "羊" "猴" "鸡" "狗" "猪")

# 星座日期范围和名称
ZODIAC_DATES=(
    "0321 0419 白羊座"
    "0420 0520 金牛座"
    "0521 0621 双子座"
    "0622 0722 巨蟹座"
    "0723 0822 狮子座"
    "0823 0922 处女座"
    "0923 1022 天秤座"
    "1023 1121 天蝎座"
    "1122 1221 射手座"
    "1222 0119 摩羯座"
    "0120 0218 水瓶座"
    "0219 0320 双鱼座"
)

# 显示标题
show_title() {
    clear
    echo -e "${CYAN}========================================${NC}"
    echo -e "${YELLOW}            算 命 大 师${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo -e "${GREEN}功能：输入出生日期，计算属相和星座${NC}"
    echo -e "${GREEN}格式：YYYY-MM-DD 或 YYYY/MM/DD${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
}

# 验证日期格式
validate_date() {
    local date_str="$1"
    
    # 检查格式 YYYY-MM-DD 或 YYYY/MM/DD
    if [[ ! "$date_str" =~ ^[0-9]{4}[-/][0-9]{2}[-/][0-9]{2}$ ]]; then
        echo -e "${RED}错误：日期格式不正确！请使用 YYYY-MM-DD 或 YYYY/MM/DD 格式${NC}"
        return 1
    fi
    
    # 提取年、月、日
    local year=$(echo "$date_str" | sed 's/[-/]/ /g' | awk '{print $1}')
    local month=$(echo "$date_str" | sed 's/[-/]/ /g' | awk '{print $2}')
    local day=$(echo "$date_str" | sed 's/[-/]/ /g' | awk '{print $3}')
    
    # 去除前导0
    year=$((10#$year))
    month=$((10#$month))
    day=$((10#$day))
    
    # 检查年份范围（1900-2100）
    if [ $year -lt 1900 ] || [ $year -gt 2100 ]; then
        echo -e "${RED}错误：年份超出范围（1900-2100）${NC}"
        return 1
    fi
    
    # 检查月份
    if [ $month -lt 1 ] || [ $month -gt 12 ]; then
        echo -e "${RED}错误：月份必须在1-12之间${NC}"
        return 1
    fi
    
    # 检查日期（简单验证）
    if [ $day -lt 1 ] || [ $day -gt 31 ]; then
        echo -e "${RED}错误：日期无效${NC}"
        return 1
    fi
    
    # 更精确的日期验证（考虑月份天数）
    case $month in
        1|3|5|7|8|10|12)
            if [ $day -gt 31 ]; then
                echo -e "${RED}错误：${month}月最多31天${NC}"
                return 1
            fi
            ;;
        4|6|9|11)
            if [ $day -gt 30 ]; then
                echo -e "${RED}错误：${month}月最多30天${NC}"
                return 1
            fi
            ;;
        2)
            # 闰年判断
            if [ $((year % 4)) -eq 0 ] && [ $((year % 100)) -ne 0 ] || [ $((year % 400)) -eq 0 ]; then
                if [ $day -gt 29 ]; then
                    echo -e "${RED}错误：${year}年是闰年，2月最多29天${NC}"
                    return 1
                fi
            else
                if [ $day -gt 28 ]; then
                    echo -e "${RED}错误：${year}年不是闰年，2月最多28天${NC}"
                    return 1
                fi
            fi
            ;;
    esac
    
    return 0
}

# 计算属相（生肖）
calculate_zodiac() {
    local year=$1
    # 生肖以1900年（鼠年）为基准
    local index=$(((year - 1900) % 12))
    echo "${ZODIAC_ANIMALS[$index]}"
}

# 计算星座
calculate_constellation() {
    local month=$1
    local day=$2
    
    # 组合月份和日期为MMDD格式
    local date_num=$((month * 100 + day))
    
    for zodiac_info in "${ZODIAC_DATES[@]}"; do
        local start_date=$(echo $zodiac_info | awk '{print $1}')
        local end_date=$(echo $zodiac_info | awk '{print $2}')
        local name=$(echo $zodiac_info | awk '{print $3}')
        
        # 处理跨年的星座（摩羯座）
        if [ "$name" == "摩羯座" ]; then
            if [ $date_num -ge $start_date ] || [ $date_num -le $end_date ]; then
                echo "$name"
                return
            fi
        else
            if [ $date_num -ge $start_date ] && [ $date_num -le $end_date ]; then
                echo "$name"
                return
            fi
        fi
    done
    
    echo "未知"
}

# 主程序
main() {
    while true; do
        show_title
        
        echo -e "${BLUE}请输入出生日期（输入 q 退出）：${NC}"
        read -p "> " input_date
        
        # 检查是否退出
        if [ "$input_date" == "q" ] || [ "$input_date" == "Q" ] || [ "$input_date" == "退出" ]; then
            echo -e "${YELLOW}感谢使用算命大师，再见！${NC}"
            break
        fi
        
        # 验证日期
        if validate_date "$input_date"; then
            # 提取年、月、日
            local year=$(echo "$input_date" | sed 's/[-/]/ /g' | awk '{print $1}')
            local month=$(echo "$input_date" | sed 's/[-/]/ /g' | awk '{print $2}')
            local day=$(echo "$input_date" | sed 's/[-/]/ /g' | awk '{print $3}')
            
            year=$((10#$year))
            month=$((10#$month))
            day=$((10#$day))
            
            # 计算结果
            local zodiac=$(calculate_zodiac $year)
            local constellation=$(calculate_constellation $month $day)
            
            # 显示结果
            echo -e "\n${CYAN}========================================${NC}"
            echo -e "${GREEN}         算命结果${NC}"
            echo -e "${CYAN}========================================${NC}"
            echo -e "${YELLOW}出生日期：${NC}${year}年${month}月${day}日"
            echo -e "${YELLOW}生    肖：${NC}${zodiac}"
            echo -e "${YELLOW}星    座：${NC}${constellation}"
            echo -e "${CYAN}========================================${NC}"
            
            # 显示一些有趣的信息
            case $zodiac in
                "鼠") echo -e "${BLUE}特点：聪明、机灵、适应力强${NC}" ;;
                "牛") echo -e "${BLUE}特点：勤奋、踏实、有耐心${NC}" ;;
                "虎") echo -e "${BLUE}特点：勇敢、自信、有领导力${NC}" ;;
                "兔") echo -e "${BLUE}特点：温和、谨慎、人缘好${NC}" ;;
                "龙") echo -e "${BLUE}特点：强大、幸运、有魅力${NC}" ;;
                "蛇") echo -e "${BLUE}特点：智慧、神秘、有直觉${NC}" ;;
                "马") echo -e "${BLUE}特点：自由、活力、热爱冒险${NC}" ;;
                "羊") echo -e "${BLUE}特点：温和、善良、有创意${NC}" ;;
                "猴") echo -e "${BLUE}特点：聪明、机智、好奇心强${NC}" ;;
                "鸡") echo -e "${BLUE}特点：勤奋、守时、有条理${NC}" ;;
                "狗") echo -e "${BLUE}特点：忠诚、诚实、有责任感${NC}" ;;
                "猪") echo -e "${BLUE}特点：善良、真诚、有福气${NC}" ;;
            esac
            echo -e "${CYAN}========================================${NC}"
        fi
        
        echo ""
        echo -e "${BLUE}按回车键继续...${NC}"
        read -p ""
    done
}

# 运行主程序
main