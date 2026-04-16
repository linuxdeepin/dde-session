#!/bin/bash
# DDE 内存分析脚本
# 用法：sudo ./analyze-memory.sh [--report]
# 输出：DDE各进程内存占用情况，便于针对性优化
# 注意：需要 root 权限以读取所有进程的 PSS 信息

set +e

# 检查是否以 root 用户运行
if [ "$(id -u)" -ne 0 ]; then
    echo "错误：此脚本需要 root 权限运行"
    echo "用法：sudo $0 [--report]"
    exit 1
fi

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_header() {
    echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}\n"
}

print_section() {
    echo -e "\n${YELLOW}─── $1 ───${NC}\n"
}

# 格式化内存大小
format_memory() {
    local kb="$1"
    [ -z "$kb" ] || [ "$kb" = "0" ] && { echo "-"; return; }
    [ "$kb" -ge 1048576 ] && { echo "$(echo "scale=2; $kb/1048576" | bc)G"; return; }
    [ "$kb" -ge 1024 ] && { echo "$((kb / 1024))M"; return; }
    echo "${kb}K"
}

# 获取进程 RSS (KB)
get_pid_rss() {
    [ -f "/proc/$1/status" ] && awk '/VmRSS:/ {print $2}' /proc/$1/status 2>/dev/null || echo 0
}

# 获取进程共享内存 (KB)
get_pid_shared() {
    [ -f "/proc/$1/status" ] && awk '/RssFile:/ {print $2}' /proc/$1/status 2>/dev/null || echo 0
}

# 获取进程 PSS (KB) - 优先使用 smaps_rollup，性能更好
get_pid_pss() {
    local pss=0
    # 优先使用 smaps_rollup (Linux 4.14+)，内核直接计算，性能极佳
    if [ -f "/proc/$1/smaps_rollup" ]; then
        pss=$(awk '/^Pss:/ {print $2}' /proc/$1/smaps_rollup 2>/dev/null)
    fi
    # 降级：使用 smaps 遍历累加
    if [ -z "$pss" ] || [ "$pss" = "0" ]; then
        if [ -f "/proc/$1/smaps" ]; then
            pss=$(awk '/^Pss:/ {sum += $2} END {print sum}' /proc/$1/smaps 2>/dev/null)
        fi
    fi
    echo "${pss:-0}"
}

print_header "DDE 内存占用分析"

echo "分析时间: $(date '+%Y-%m-%d %H:%M:%S')"
echo "系统版本: $(cat /etc/os-release 2>/dev/null | grep PRETTY_NAME | cut -d= -f2 | tr -d '"')"

# 获取系统总内存（提前计算，后续部分会用到）
MEM_TOTAL=$(awk '/MemTotal:/ {print $2}' /proc/meminfo)
MEM_FREE=$(awk '/MemFree:/ {print $2}' /proc/meminfo)
MEM_AVAILABLE=$(awk '/MemAvailable:/ {print $2}' /proc/meminfo)
MEM_USED=$((MEM_TOTAL - MEM_AVAILABLE))

# 获取 DDE 进程内存信息
dde_tmpfile=$(mktemp)

# 使用 awk 精确匹配命令列（第11列起），避免用户名列干扰，同时排除脚本自身
ps aux | awk -v self_pid="$$" 'NR>1 && $2!=self_pid {cmd=""; for(i=11;i<=NF;i++) cmd=cmd $i " "; if(cmd ~ /(dde|deepin|service-manager|tray|kwin)/ && cmd !~ /(uos-license-agent|deepin-anything|deepin-elf-verify)/) print $2}' | while read pid; do
    if [ -d "/proc/$pid" ]; then
        rss=$(get_pid_rss "$pid")
        shared=$(get_pid_shared "$pid")
        pss=$(get_pid_pss "$pid")
        total=$((rss + shared))
        comm=$(cat /proc/$pid/comm 2>/dev/null || echo "unknown")
        echo "${total}|${rss}|${shared}|${pss}|${pid}|${comm}"
    fi
done | sort -t'|' -k1 -rn > "$dde_tmpfile"

DDE_TOTAL_PSS=0
DDE_TOTAL_RSS=0
DDE_PROCESS_COUNT=0

while IFS='|' read total rss shared pss pid comm; do
    DDE_TOTAL_PSS=$((DDE_TOTAL_PSS + pss))
    DDE_TOTAL_RSS=$((DDE_TOTAL_RSS + rss))
    DDE_PROCESS_COUNT=$((DDE_PROCESS_COUNT + 1))
done < "$dde_tmpfile"

DDE_PERCENT=$(echo "scale=2; $DDE_TOTAL_PSS * 100 / $MEM_TOTAL" | bc)
DDE_PSS_PERCENT=$(echo "scale=2; $DDE_TOTAL_PSS * 100 / $MEM_USED" | bc)

# ============================================================
# 第一部分：系统内存概览
# ============================================================

print_section "1. 系统内存概览"

MEM_TOTAL_STR=$(format_memory $MEM_TOTAL)
MEM_USED_STR=$(format_memory $MEM_USED)
MEM_AVAILABLE_STR=$(format_memory $MEM_AVAILABLE)
MEM_USED_PCT=$(echo "scale=1; $MEM_USED * 100 / $MEM_TOTAL" | bc)
MEM_AVAILABLE_PCT=$(echo "scale=1; $MEM_AVAILABLE * 100 / $MEM_TOTAL" | bc)

echo ""
echo "  项目              大小        占比"
echo "  ----------------  ----------  --------"
echo "  物理内存总量      $MEM_TOTAL_STR        100%"
echo "  已使用            $MEM_USED_STR        ${MEM_USED_PCT}%"
echo "  可用内存          $MEM_AVAILABLE_STR        ${MEM_AVAILABLE_PCT}%"

# ============================================================
# 第二部分：系统进程内存占用列表（按内存排序）
# ============================================================

print_section "2. 系统进程内存占用列表 (Top 100, 按内存排序)"

echo ""
echo "  PID      用户         RSS      共享      PSS      进程名                命令行"
echo "  -------- ------------ -------- -------- -------- -------------------- --------------------------------------------------------------------------"

# 获取系统所有进程，按内存排序
ps aux --sort=-rss | head -101 | tail -100 | while read -r user pid cpu mem vsz rss_kb tty stat start time comm; do
    [ -z "$pid" ] && continue

    if [ -d "/proc/$pid" ]; then
        shared_kb=$(get_pid_shared "$pid")
        pss_kb=$(get_pid_pss "$pid")

        # 格式化内存
        rss_str=$(format_memory "$rss_kb")
        shared_str=$(format_memory "$shared_kb")
        pss_str=$(format_memory "$pss_kb")

        # 获取进程名
        proc_name=$(cat /proc/$pid/comm 2>/dev/null || echo "$comm" | awk '{print $1}')
        proc_name=$(echo "$proc_name" | cut -c1-20)

        # 获取完整命令行
        cmdline=$(tr '\0' ' ' < /proc/$pid/cmdline 2>/dev/null | sed 's/ $//')
        if [ -z "$cmdline" ]; then
            # 如果 cmdline 为空，尝试读取 exe 链接
            cmdline=$(readlink /proc/$pid/exe 2>/dev/null)
        fi
        [ -z "$cmdline" ] && cmdline=$(cat /proc/$pid/comm 2>/dev/null)

        # 截取命令行到90个字符
        cmdline=$(echo "$cmdline" | cut -c1-90)

        # 截取用户名
        user_short=$(echo "$user" | cut -c1-12)

        printf "  %-8s %-12s %-8s %-8s %-8s %-20s %s\n" "$pid" "$user_short" "$rss_str" "$shared_str" "$pss_str" "$proc_name" "$cmdline"
    fi
done

echo ""
echo "  注：按内存占用排序，显示系统所有进程的前 100 个"

# ============================================================
# 第三部分：DDE 进程内存占用
# ============================================================

print_section "3. DDE 进程内存占用"

echo ""
echo "  PID       RSS      共享      PSS      进程名                命令行"
echo "  -------- -------- -------- -------- -------------------- --------------------------------------------------------------------------"

# 显示所有 DDE 进程
while IFS='|' read total rss shared pss pid comm; do
    cmdline=$(tr '\0' ' ' < /proc/$pid/cmdline 2>/dev/null | cut -c1-90)
    [ -z "$cmdline" ] && cmdline=$comm
    comm=$(echo "$comm" | cut -c1-20)
    printf "  %-8s %-8s %-8s %-8s %-20s %s\n" "$pid" "$(format_memory $rss)" "$(format_memory $shared)" "$(format_memory $pss)" "$comm" "$cmdline"
done < "$dde_tmpfile"

	# DDE 内存汇总
echo ""
echo "  项目              数值"
echo "  ----------------  ----------------"
echo "  DDE 进程数        $DDE_PROCESS_COUNT"
echo "  DDE 内存(PSS)     $(format_memory $DDE_TOTAL_PSS) (占系统 ${DDE_PERCENT}%)"
echo "  占已用内存        ${DDE_PSS_PERCENT}%"
echo "  ────────────────────────────────────"
echo "  DDE 内存(RSS)     $(format_memory $DDE_TOTAL_RSS)"
echo ""
echo "  【指标说明】"
echo "  RSS  - 进程物理内存总量，已包含共享内存（私有+共享库+共享内存段）"
echo "  共享  - RSS 中来自文件映射的部分（如 .so 共享库），是 RSS 的子集"
echo "  PSS  - 按比例分配共享内存后的实际物理占用，多进程累加不会重复计算"
echo ""
echo "  【示例】某共享库 10M 被 3 个进程使用："
echo "         每个进程 RSS 都包含完整 10M，累加得 30M（重复计算）"
echo "         每个进程 PSS 只算 10M÷3≈3.3M，累加得 10M（真实占用）"

# ============================================================
# 第四部分：总结
# ============================================================

print_section "4. 分析总结"

echo ""
echo "  【内存占用 Top 5 DDE 进程】"
echo ""

# 获取 Top 5 进程信息
top5_file=$(mktemp)
head -5 "$dde_tmpfile" | while IFS='|' read total rss shared pss pid comm; do
    cmdline=$(tr '\0' ' ' < /proc/$pid/cmdline 2>/dev/null | cut -c1-50)
    echo "${total}|${rss}|${shared}|${pss}|${pid}|${comm}|${cmdline}"
done > "$top5_file"

i=1
while IFS='|' read total rss shared pss pid comm cmdline; do
    pss_str=$(format_memory $pss)
    [ "$pss_str" = "-" ] && pss_str="N/A"
    printf "  %d. %-25s RSS: %s, 共享: %s, PSS: %s\n" "$i" "$comm" "$(format_memory $rss)" "$(format_memory $shared)" "$pss_str"
    i=$((i + 1))
done < "$top5_file"

rm -f "$top5_file"
rm -f "$dde_tmpfile"

echo ""
echo "  【优化建议】"
echo ""

suggestions=()

# 检查任务栏
SHELL_PID=$(pgrep -f "dde-shell.*-C DDE" | head -1)
if [ -n "$SHELL_PID" ]; then
    SHELL_RSS=$(get_pid_rss "$SHELL_PID")
    if [ "$SHELL_RSS" -gt 204800 ]; then
        suggestions+=("${RED}[P0]${NC} 任务栏进程内存超过 200M ($(format_memory $SHELL_RSS))，建议检查插件内存泄漏或延迟加载非必要组件")
    elif [ "$SHELL_RSS" -gt 102400 ]; then
        suggestions+=("${YELLOW}[P1]${NC} 任务栏进程内存超过 100M ($(format_memory $SHELL_RSS))，建议检查插件内存占用")
    fi
fi

# 检查 dde-daemon
DAEMON_PID=$(pgrep -f "dde-session-daemon" | head -1)
if [ -n "$DAEMON_PID" ]; then
    DAEMON_RSS=$(get_pid_rss "$DAEMON_PID")
    if [ "$DAEMON_RSS" -gt 102400 ]; then
        suggestions+=("${YELLOW}[P1]${NC} dde-daemon 内存超过 100M ($(format_memory $DAEMON_RSS))，建议检查是否存在内存泄漏")
    fi
fi

# 检查总内存占比
if [ "$(echo "$DDE_PERCENT > 10" | bc)" -eq 1 ]; then
    suggestions+=("${YELLOW}[P1]${NC} DDE 总内存占比超过 10% (${DDE_PERCENT}%)，建议优化内存使用")
elif [ "$(echo "$DDE_PERCENT > 5" | bc)" -eq 1 ]; then
    suggestions+=("${YELLOW}[P2]${NC} DDE 总内存占比 ${DDE_PERCENT}%，处于中等水平")
fi

# 检查进程数
if [ "$DDE_PROCESS_COUNT" -gt 40 ]; then
    suggestions+=("${YELLOW}[P2]${NC} DDE 相关进程数较多($DDE_PROCESS_COUNT)，建议合并部分功能或延迟启动非必要服务")
fi

# 检查系统内存压力
MEM_USED_PERCENT=$((MEM_USED * 100 / MEM_TOTAL))
if [ "$MEM_USED_PERCENT" -gt 80 ]; then
    suggestions+=("${RED}[P0]${NC} 系统内存使用率超过 80% (${MEM_USED_PERCENT}%)，存在内存压力")
elif [ "$MEM_USED_PERCENT" -gt 60 ]; then
    suggestions+=("${YELLOW}[P1]${NC} 系统内存使用率 ${MEM_USED_PERCENT}%，建议关注内存使用")
fi

if [ ${#suggestions[@]} -eq 0 ]; then
    echo -e "  ${GREEN}✓ DDE 内存占用在合理范围内${NC}"
else
    for suggestion in "${suggestions[@]}"; do
        echo -e "  - $suggestion"
    done
fi

echo ""
echo "═══════════════════════════════════════════════════════════"
echo "内存分析报告生成完毕"
echo "═══════════════════════════════════════════════════════════"

# ============================================================
# 生成 Markdown 报告
# ============================================================

if [ "$1" = "--report" ]; then
    REPORT_FILE="memory-analysis-$(date +%Y%m%d-%H%M%S).md"
    cat > "$REPORT_FILE" << EOF
# DDE 内存占用分析报告

> 生成时间：$(date '+%Y-%m-%d %H:%M:%S')
> 系统版本：$(cat /etc/os-release 2>/dev/null | grep PRETTY_NAME | cut -d= -f2 | tr -d '"')

## 1. 系统内存概览

| 项目 | 大小 | 占比 |
|------|------|------|
| 物理内存总量 | $(format_memory $MEM_TOTAL) | 100% |
| 已使用 | $(format_memory $MEM_USED) | $(echo "scale=1; $MEM_USED * 100 / $MEM_TOTAL" | bc)% |
| 可用内存 | $(format_memory $MEM_AVAILABLE) | $(echo "scale=1; $MEM_AVAILABLE * 100 / $MEM_TOTAL" | bc)% |

## 2. DDE 内存占用统计

| 项目 | 数值 |
|------|------|
| DDE 进程数 | $DDE_PROCESS_COUNT |
| DDE 内存(PSS) | $(format_memory $DDE_TOTAL_PSS) (占系统 ${DDE_PERCENT}%) |
| 占已用内存 | ${DDE_PSS_PERCENT}% |
| DDE 内存(RSS) | $(format_memory $DDE_TOTAL_RSS) |

> **指标说明**
> - **RSS**：进程物理内存总量，已包含共享内存（私有+共享库+共享内存段）
> - **共享**：RSS 中来自文件映射的部分（如 .so 共享库），是 RSS 的子集
> - **PSS**：按比例分配共享内存后的实际物理占用，多进程累加不会重复计算
>
> **示例**：某共享库 10M 被 3 个进程使用时，每个进程 RSS 都包含完整 10M（累加得 30M），而 PSS 只算 3.3M（累加得 10M）。

## 3. 优化建议

EOF

    for suggestion in "${suggestions[@]}"; do
        # 先用 printf '%b' 渲染转义序列，再清理颜色代码
        clean=$(printf '%b' "$suggestion" | sed 's/\x1b\[[0-9;]*m//g')
        echo "- $clean" >> "$REPORT_FILE"
    done

    echo "" >> "$REPORT_FILE"
    echo "报告已保存到：$REPORT_FILE"
fi
