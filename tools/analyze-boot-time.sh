#!/bin/bash
# DDE 启动性能分析脚本
# 用法：./analyze-boot-time.sh [--report]
# 输出：systemd 用户会话启动时间线、关键服务耗时、优化建议
#
# 选项：
#   --report  生成 Markdown 格式的分析报告文件

# 不使用 set -e，因为某些检查命令可能失败但不应中断脚本
set +e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 格式化输出函数
print_header() {
    echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}\n"
}

print_section() {
    echo -e "\n${YELLOW}─── $1 ───${NC}\n"
}

print_service() {
    printf "  %-50s %10s\n" "$1" "$2"
}

print_conclusion() {
    echo -e "\n${GREEN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}分析结论${NC}"
    echo -e "${GREEN}═══════════════════════════════════════════════════════════${NC}\n"
}

# 获取 systemd 用户服务的时间戳（微秒）
get_timestamp() {
    local service="$1"
    local prop="$2"
    systemctl --user show "$service" -p "$prop" --no-pager 2>/dev/null | cut -d= -f2
}

# 计算时间差（毫秒）
calc_ms() {
    local start="$1"
    local end="$2"
    if [ "$start" = "0" ] || [ "$end" = "0" ] || [ -z "$start" ] || [ -z "$end" ]; then
        echo "N/A"
        return
    fi
    echo $(( (end - start) / 1000 ))
}

# 计算相对于基准的时间（毫秒）
calc_rel_ms() {
    local base="$1"
    local ts="$2"
    if [ "$base" = "0" ] || [ "$ts" = "0" ] || [ -z "$base" ] || [ -z "$ts" ]; then
        echo "N/A"
        return
    fi
    echo $(( (ts - base) / 1000 ))
}

print_header "DDE 启动性能分析"

# ============================================================
# 第一步：收集所有时间戳数据
# ============================================================

# 辅助函数：从 target.wants 目录读取服务列表
get_target_services() {
    local target="$1"
    local dir="/usr/lib/systemd/user/${target}.wants"
    if [ -d "$dir" ]; then
        ls "$dir" 2>/dev/null | sed 's/\.service$//' | sort
    fi
}

# 辅助函数：获取服务的时间戳
get_svc_timestamps() {
    local svc="$1"
    local start=$(get_timestamp "$svc" "ExecMainStartTimestampMonotonic")
    local ready=$(get_timestamp "$svc" "ActiveEnterTimestampMonotonic")
    echo "$start:$ready"
}

# Target 只有就绪时刻
PRE_READY=$(get_timestamp "dde-session-pre.target" "ActiveEnterTimestampMonotonic")
CORE_READY=$(get_timestamp "dde-session-core.target" "ActiveEnterTimestampMonotonic")
INIT_READY=$(get_timestamp "dde-session-initialized.target" "ActiveEnterTimestampMonotonic")
SESSION_READY=$(get_timestamp "dde-session.target" "ActiveEnterTimestampMonotonic")

# 找出最早启动的服务作为基准 T=0（使用启动时刻，从 pre 阶段服务中找）
BASELINE=""
BASELINE_NAME=""

# Pre 阶段的额外服务（不在 target.wants 中，但需要用于基准检测）
PRE_EXTRA_SERVICES="dde-session-manager"

# 检查所有 Pre 阶段服务的启动时刻
for svc in $(get_target_services "dde-session-pre.target") $PRE_EXTRA_SERVICES; do
    ts=$(get_svc_timestamps "${svc}.service")
    start="${ts%%:*}"
    if [ -n "$start" ] && [ "$start" != "0" ]; then
        if [ -z "$BASELINE" ] || [ "$start" -lt "$BASELINE" ]; then
            BASELINE=$start
            BASELINE_NAME=$svc
        fi
    fi
done

if [ -z "$BASELINE" ] || [ "$BASELINE" = "0" ]; then
    echo -e "${RED}错误：无法获取服务时间戳，可能 DDE 未运行${NC}"
    exit 1
fi

# 计算关键时间点（相对于基准启动时刻）
PRE_REL=$(calc_rel_ms $BASELINE $PRE_READY)
CORE_REL=$(calc_rel_ms $BASELINE $CORE_READY)
INIT_REL=$(calc_rel_ms $BASELINE $INIT_READY)
SESSION_REL=$(calc_rel_ms $BASELINE $SESSION_READY)

# 辅助函数：获取服务的时间戳
get_svc_timestamps() {
    local svc="$1"
    local start=$(get_timestamp "$svc" "ExecMainStartTimestampMonotonic")
    local ready=$(get_timestamp "$svc" "ActiveEnterTimestampMonotonic")
    echo "$start:$ready"
}

# 辅助函数：打印服务信息（显示启动时刻、就绪时刻、耗时）
print_svc() {
    local name="$1"
    local start_ts="$2"
    local ready_ts="$3"
    local note="$4"

    # 计算相对时间
    local start_rel=$(calc_rel_ms $BASELINE $start_ts)
    local ready_rel=$(calc_rel_ms $BASELINE $ready_ts)
    local duration=$(calc_ms $start_ts $ready_ts)

    # 格式化时间字符串
    local start_str ready_str
    if [ "$name" = "$BASELINE_NAME" ]; then
        start_str="T+0ms"
        note="基准"
    else
        start_str="T+${start_rel}ms"
    fi
    ready_str="T+${ready_rel}ms"

    printf "  %-40s %10s %10s %8s %s\n" "$name" "$start_str" "$ready_str" "$duration" "$note"
}

# 辅助函数：打印 Target（只有就绪时刻）
print_target() {
    local name="$1"
    local ready_ts="$2"
    local note="$3"
    local ready_rel=$(calc_rel_ms $BASELINE $ready_ts)
    printf "  %-40s %10s %10s %8s %s\n" "$name" "--" "T+${ready_rel}ms" "--" "$note"
}

# 辅助函数：打印一个阶段的所有服务
print_phase_services() {
    local target="$1"
    local ready_ts="$2"
    local note="$3"
    local extra_svcs="$4"  # 额外的服务（不在 target.wants 中）

    local services=$(get_target_services "$target")

    # 收集服务信息到临时文件，便于排序
    local tmpfile=$(mktemp)
    local baseline_file=$(mktemp)

    for svc in $services $extra_svcs; do
        [ -z "$svc" ] && continue
        local ts=$(get_svc_timestamps "${svc}.service")
        local start="${ts%%:*}"
        local ready="${ts#*:}"

        # 简化显示名称
        local display_name="$svc"
        case "$svc" in
            "dde-session@x11") display_name="dde-session@x11 (kwin)" ;;
            "dde-shell@DDE") display_name="dde-shell@DDE (任务栏)" ;;
            "dde-shell-plugin@org.deepin.ds.desktop") display_name="dde-shell-plugin@desktop (桌面)" ;;
        esac

        # 检查是否是基准服务
        if [ "$svc" = "$BASELINE_NAME" ]; then
            echo "$display_name|$start|$ready" > "$baseline_file"
        else
            echo "${start:-999999999}|$display_name|$start|$ready" >> "$tmpfile"
        fi
    done

    # 先打印基准服务（如果在这个阶段）
    if [ -s "$baseline_file" ]; then
        local line=$(cat "$baseline_file")
        local name="${line%%|*}"
        local rest="${line#*|}"
        local start="${rest%%|*}"
        local ready="${rest#*|}"
        print_svc "$name" "$start" "$ready" ""
    fi

    # 按启动时间排序并打印其他服务
    sort -t'|' -k1 -n "$tmpfile" | while IFS='|' read sortkey name start ready; do
        print_svc "$name" "$start" "$ready" ""
    done

    # 打印 target
    print_target "→ $target" "$ready_ts" "$note"

    rm -f "$tmpfile" "$baseline_file"
}

# ============================================================
# 第二步：显示启动时刻表格（最重要的信息）
# ============================================================

print_section "1. 启动时刻总览"

echo "基准服务: $BASELINE_NAME (最早启动的服务)"
echo ""

printf "  %-40s %10s %10s %8s %s\n" "服务" "启动" "就绪" "耗时" "备注"
printf "  %-40s %10s %10s %8s %s\n" "────────────────────────────────────" "──────────" "──────────" "────────" "──────────"

# Pre 阶段（额外包含 dde-session-manager，用于基准检测）
echo "  【dde-session-pre.target 阶段】"
print_phase_services "dde-session-pre.target" "$PRE_READY" "关键路径 1" \
    "dde-session-manager"

echo ""
echo "  【dde-session-core.target 阶段】"
print_phase_services "dde-session-core.target" "$CORE_READY" "关键路径 2" ""

echo ""
echo "  【dde-session-initialized.target 阶段】"
print_phase_services "dde-session-initialized.target" "$INIT_READY" "关键路径 3" ""

echo ""
print_target "→ dde-session.target" "$SESSION_READY" "完成"

# ============================================================
# 第三步：其他分析信息
# ============================================================

print_section "2. systemd-analyze blame (Top 10, 仅显示可信时间)"

echo ""
echo "  仅显示 Type=notify/dbus 的服务（oneshot/simple 时间可能有 ExecStartPre 干扰，已过滤）"
echo ""
printf "  %-50s %12s %10s\n" "服务名称" "耗时" "类型"
printf "  %-50s %12s %10s\n" "──────────────────────────────────────────────────" "────────────" "──────────"

# 获取所有服务并只保留 Type=notify/dbus（oneshot 可能有 ExecStartPre 干扰）
tmpfile=$(mktemp)
systemd-analyze --user blame 2>/dev/null | while read time service; do
    svc_type=$(systemctl --user show "$service" -p Type --no-pager 2>/dev/null | cut -d= -f2)
    # 只保留 notify 和 dbus 类型（这些类型的 blame 时间才是真实的启动耗时）
    if [ "$svc_type" = "notify" ] || [ "$svc_type" = "dbus" ]; then
        echo "$time|$service|$svc_type"
    fi
done > "$tmpfile"

# 显示前 10 个
head -10 "$tmpfile" | while IFS='|' read time service svc_type; do
    printf "  %-50s %12s %10s\n" "$service" "$time" "${svc_type:-unknown}"
done

rm -f "$tmpfile"

echo ""
echo "  注：Type=notify/dbus 的服务，时间是进程启动到通知就绪的真实耗时"

print_section "3. systemd 用户会话进程列表 (Top 50)"

echo ""
printf "  %-8s %-8s %-10s %-7s %-40s %s\n" "PID" "PPID" "启动时间" "内存" "服务名" "命令行"
printf "  %-8s %-8s %-10s %-7s %-40s %s\n" "--------" "--------" "----------" "-------" "----------------------------------------" "----------------------------------------"

# 获取所有 systemd --user 管理的运行中服务
tmpfile=$(mktemp)

# 获取服务的 MainPID 和启动时间
for svc in $(systemctl --user list-units --type=service --state=running --no-pager 2>/dev/null | awk '{print $1}'); do
    main_pid=$(systemctl --user show "$svc" -p MainPID --no-pager 2>/dev/null | cut -d= -f2)
    start_ts=$(systemctl --user show "$svc" -p ExecMainStartTimestampMonotonic --no-pager 2>/dev/null | cut -d= -f2)

    if [ -n "$main_pid" ] && [ "$main_pid" != "0" ]; then
        # 计算相对启动时间
        start_rel=$(calc_rel_ms $BASELINE $start_ts)
        echo "${start_rel:-999999999}|${main_pid}|${svc}" >> "$tmpfile"
    fi
done

# 按启动时间排序并显示前 50 个
sort -t'|' -k1 -n "$tmpfile" | head -50 | while IFS='|' read start_rel pid svc; do
    [ -z "$pid" ] || [ "$pid" = "0" ] && continue

    # 获取进程信息
    if [ -d "/proc/$pid" ]; then
        ppid=$(awk '/PPid:/ {print $2}' /proc/$pid/status 2>/dev/null)
        cmdline=$(tr '\0' ' ' < /proc/$pid/cmdline 2>/dev/null | sed 's/ $//')
        mem_kb=$(awk '/VmRSS:/ {print $2}' /proc/$pid/status 2>/dev/null)

        # 格式化内存
        if [ -n "$mem_kb" ]; then
            if [ "$mem_kb" -gt 1024 ]; then
                mem_str="$((mem_kb / 1024))M"
            else
                mem_str="${mem_kb}K"
            fi
        else
            mem_str="-"
        fi

        # 格式化启动时间
        if [ "$start_rel" = "0" ]; then
            time_str="T+0ms"
        elif [ "$start_rel" != "999999999" ]; then
            time_str="T+${start_rel}ms"
        else
            time_str="-"
        fi

        # 提取服务名（去掉 .service 后缀，并截断到 40 字符）
        svc_short=$(echo "$svc" | sed 's/\.service$//' | cut -c1-40)

        # 如果 cmdline 为空，尝试读取 comm
        if [ -z "$cmdline" ]; then
            cmdline=$(cat /proc/$pid/comm 2>/dev/null)
        fi

        printf "  %-8s %-8s %-10s %-7s %-40s %s\n" "$pid" "${ppid:- -}" "$time_str" "$mem_str" "$svc_short" "$cmdline"
    fi
done

rm -f "$tmpfile"

echo ""
echo "  注：按启动时间排序，仅显示运行中的服务进程，服务名超过 40 字符会被截断"

print_section "4. dde-shell 内部耗时分析"

# 获取最近一次启动的时间范围（使用 systemd 的启动时间）
SESSION_START_TS=$(get_timestamp "dde-session-manager.service" "ActiveEnterTimestampMonotonic")
if [ -z "$SESSION_START_TS" ] || [ "$SESSION_START_TS" = "0" ]; then
    echo "  无法获取会话启动时间，跳过日志分析"
else
    # 计算启动时间（当前时间 - 单调时间差）
    CURRENT_MONOTONIC=$(date +%s%N)
    CURRENT_REALTIME=$(date +%s)
    # 简化：使用 --boot 0 获取本次启动的日志
    
    echo "  分析本次启动的日志..."
    echo ""
    
    # 分析任务栏进程 (dde-shell@DDE)
    echo "  【任务栏进程 dde-shell@DDE】"
    SHELL_PID=$(systemctl --user show dde-shell@DDE.service -p MainPID --no-pager 2>/dev/null | cut -d= -f2)
    if [ -n "$SHELL_PID" ] && [ "$SHELL_PID" != "0" ]; then
        echo "    PID: $SHELL_PID"
        
        # 提取关键日志（使用 --boot 0 获取本次启动）
        SHELL_FIRST_LOG=$(journalctl --user -u dde-shell@DDE.service --boot 0 -o short-precise 2>/dev/null | grep -v "^--" | grep "dde-shell\[" | head -1)
        SHELL_DCONFIG_ERRORS=$(journalctl --user -u dde-shell@DDE.service --boot 0 2>/dev/null | grep -c "acquire failed" || echo "0")
        SHELL_NOTIFY_LOGS=$(journalctl --user -u dde-shell@DDE.service --boot 0 2>/dev/null | grep -i "notification" | head -5)
        
        if [ -n "$SHELL_FIRST_LOG" ]; then
            FIRST_LOG_SHORT=$(echo "$SHELL_FIRST_LOG" | cut -c1-100)
            echo "    首条日志: ${FIRST_LOG_SHORT}..."
        fi
        
        if [ "$SHELL_DCONFIG_ERRORS" -gt 0 ] 2>/dev/null; then
            echo -e "    ${YELLOW}⚠  DConfig acquire 失败: ${SHELL_DCONFIG_ERRORS} 次${NC}"
        else
            echo -e "    ${GREEN}✓ 无 DConfig 错误${NC}"
        fi
        
        # 检测 notificationserver 相关日志
        if echo "$SHELL_NOTIFY_LOGS" | grep -q "NotificationManager\|DBAccessor"; then
            echo -e "    ${YELLOW}⚠  检测到 NotificationManager/DBAccessor 日志（可能在 init 中执行重操作）${NC}"
        fi
    else
        echo "    服务未运行或无法获取进程信息"
    fi
    
    echo ""
    
    # 分析桌面进程 (dde-shell-plugin@org.deepin.ds.desktop)
    echo "  【桌面进程 dde-shell-plugin@org.deepin.ds.desktop】"
    DESKTOP_PID=$(systemctl --user show dde-shell-plugin@org.deepin.ds.desktop.service -p MainPID --no-pager 2>/dev/null | cut -d= -f2)
    if [ -n "$DESKTOP_PID" ] && [ "$DESKTOP_PID" != "0" ]; then
        echo "    PID: $DESKTOP_PID"
        
        # 提取关键日志
        DESKTOP_FIRST_LOG=$(journalctl --user -u dde-shell-plugin@org.deepin.ds.desktop.service --boot 0 -o short-precise 2>/dev/null | grep -v "^--" | grep "dde-shell-plugin\[" | head -1)
        DESKTOP_DCONFIG_ERRORS=$(journalctl --user -u dde-shell-plugin@org.deepin.ds.desktop.service --boot 0 2>/dev/null | grep -c "acquire failed" || echo "0")
        DESKTOP_NOREPLY=$(journalctl --user -u dde-shell-plugin@org.deepin.ds.desktop.service --boot 0 2>/dev/null | grep "NoReply" | head -1)
        
        if [ -n "$DESKTOP_FIRST_LOG" ]; then
            FIRST_LOG_SHORT=$(echo "$DESKTOP_FIRST_LOG" | cut -c1-100)
            echo "    首条日志: ${FIRST_LOG_SHORT}..."
        fi
        
        if [ "$DESKTOP_DCONFIG_ERRORS" -gt 0 ] 2>/dev/null; then
            echo -e "    ${YELLOW}⚠  DConfig acquire 失败: ${DESKTOP_DCONFIG_ERRORS} 次${NC}"
        else
            echo -e "    ${GREEN}✓ 无 DConfig 错误${NC}"
        fi
        
        # 检测壁纸 DBus 超时（最关键的瓶颈）
        if [ -n "$DESKTOP_NOREPLY" ]; then
            echo -e "    ${RED}✗ 检测到 DBus NoReply 超时（壁纸服务未就绪）${NC}"
            NOREPLY_SHORT=$(echo "$DESKTOP_NOREPLY" | cut -c1-100)
            echo "    详情: ${NOREPLY_SHORT}..."
            echo -e "    ${YELLOW}建议: 优先从本地配置读取壁纸，DBus 调用改为异步${NC}"
        else
            echo -e "    ${GREEN}✓ 未检测到 DBus 超时${NC}"
        fi
    else
        echo "    服务未运行或无法获取进程信息"
    fi
fi

print_section "5. 已安装的 systemd 服务状态"

# 检查实际安装的服务文件
echo "  检查系统已安装的服务配置文件..."
echo ""

# 检查 XSettings1
XSETTINGS_FILE="/usr/lib/systemd/user/org.deepin.dde.XSettings1.service"
if [ -f "$XSETTINGS_FILE" ]; then
    if grep -q "Before=org.dde.session.Daemon1" "$XSETTINGS_FILE" 2>/dev/null; then
        echo -e "  ${RED}✗${NC} $XSETTINGS_FILE"
        echo "      仍存在 Before=org.dde.session.Daemon1.service（串行依赖）"
    else
        echo -e "  ${GREEN}✓${NC} $XSETTINGS_FILE"
        echo "      已移除串行依赖"
    fi
fi

# 检查 Daemon1
DAEMON_FILE="/usr/lib/systemd/user/org.dde.session.Daemon1.service"
if [ -f "$DAEMON_FILE" ]; then
    if grep -q "Before=dde-session@x11" "$DAEMON_FILE" 2>/dev/null; then
        echo -e "  ${RED}✗${NC} $DAEMON_FILE"
        echo "      仍存在 Before=dde-session@x11.service（阻塞 kwin）"
    else
        echo -e "  ${GREEN}✓${NC} $DAEMON_FILE"
        echo "      已移除串行依赖"
    fi
fi

# 检查 dde-shell@DDE
SHELL_FILE="/usr/lib/systemd/user/dde-shell@DDE.service"
if [ -f "$SHELL_FILE" ]; then
    SHELL_TYPE_VAL=$(grep -E "^Type=" "$SHELL_FILE" 2>/dev/null | cut -d= -f2)
    if [ "$SHELL_TYPE_VAL" = "notify" ]; then
        echo -e "  ${GREEN}✓${NC} $SHELL_FILE"
        echo "      Type=notify 已配置"
    else
        echo -e "  ${YELLOW}!${NC} $SHELL_FILE"
        echo "      Type=$SHELL_TYPE_VAL，建议改为 notify"
    fi
fi

# 检查 kwin ExecStartPre
KWIN_FILE="/usr/lib/systemd/user/dde-session@x11.service"
if [ -f "$KWIN_FILE" ]; then
    if grep -q "ExecStartPre=" "$KWIN_FILE" 2>/dev/null; then
        echo -e "  ${YELLOW}!${NC} $KWIN_FILE"
        echo "      存在 ExecStartPre 命令，建议移除或移出关键路径"
    else
        echo -e "  ${GREEN}✓${NC} $KWIN_FILE"
        echo "      无 ExecStartPre 阻塞"
    fi
fi

print_section "6. 优化建议汇总"

SUGGESTIONS=()

# 检查 kwin ExecStartPre
KWIN_EXEC_PRE=$(systemctl --user show dde-session@x11.service -p ExecStartPre --no-pager 2>/dev/null)
if [[ "$KWIN_EXEC_PRE" == *"ExecStartPre"* ]] && [[ "$KWIN_EXEC_PRE" != *"ExecStartPre=" ]]; then
    SUGGESTIONS+=("${YELLOW}[P2]${NC} kwin 存在 ExecStartPre 命令，建议合并或移除（可节省 10-20ms）")
fi

# 检查 file-manager 是否阻塞 initialized
FM_BEFORE=$(systemctl --user show dde-file-manager.service -p Before --no-pager 2>/dev/null | cut -d= -f2)
if [[ "$FM_BEFORE" == *"initialized"* ]]; then
    FM_START_TS=$(get_timestamp "dde-file-manager.service" "ExecMainStartTimestampMonotonic")
    FM_READY_TS=$(get_timestamp "dde-file-manager.service" "ActiveEnterTimestampMonotonic")
    FM_DURATION=$(calc_ms $FM_START_TS $FM_READY_TS)
    SUGGESTIONS+=("${YELLOW}[P1]${NC} dde-file-manager 阻塞 initialized.target (启动耗时 ${FM_DURATION}ms)，建议移出关键路径")
fi

# 检查 dde-shell 类型
SHELL_TYPE=$(systemctl --user show dde-shell@DDE.service -p Type --no-pager 2>/dev/null | cut -d= -f2)
if [ "$SHELL_TYPE" = "simple" ]; then
    SUGGESTIONS+=("${YELLOW}[P1]${NC} dde-shell@DDE 为 Type=simple，建议改为 Type=notify + sd_notify(READY=1)")
fi

# 检查 AM 是否在 pre 阶段
AM_AFTER=$(systemctl --user show org.desktopspec.ApplicationManager1.service -p After --no-pager 2>/dev/null | cut -d= -f2)
if [[ "$AM_AFTER" == *"dde-session-pre.target"* ]]; then
    SUGGESTIONS+=("${GREEN}[P0 已完成]${NC} AM 提前到 graphical-session-pre 阶段（与 kwin 并行）")
else
    SUGGESTIONS+=("${YELLOW}[P0]${NC} AM 应提前到 graphical-session-pre.target，与 kwin 并行启动")
fi

# 从日志分析中提取的建议
if [ -n "$DESKTOP_NOREPLY" ]; then
    SUGGESTIONS+=("${RED}[P0 关键]${NC} 桌面进程存在 DBus NoReply 超时，建议优先从本地读取壁纸")
fi

if [ "$SHELL_DCONFIG_ERRORS" -gt 5 ] 2>/dev/null || [ "$DESKTOP_DCONFIG_ERRORS" -gt 5 ] 2>/dev/null; then
    SUGGESTIONS+=("${YELLOW}[P1]${NC} DConfig acquire 失败次数过多，建议修复资源安装或合并实例")
fi

# 输出建议
if [ ${#SUGGESTIONS[@]} -eq 0 ]; then
    echo -e "  ${GREEN}✓ 未发现明显的优化点，启动流程已优化良好${NC}"
else
    echo "  发现 ${#SUGGESTIONS[@]} 个优化建议："
    echo ""
    for i in "${!SUGGESTIONS[@]}"; do
        echo -e "  $((i+1)). ${SUGGESTIONS[$i]}"
    done
fi

print_conclusion

# 计算总耗时
TOTAL_TO_DESKTOP=$INIT_REL
TOTAL_TO_SESSION=$SESSION_REL

echo "  关键路径时间:"
echo "  ┌────────────────────────────────────────────────────┐"
printf "  │ pre.target:          T+%10sms                   │\n" "$PRE_REL"
printf "  │ core.target:         T+%10sms                   │\n" "$CORE_REL"
printf "  │ initialized.target:  T+%10sms  ← 用户看到桌面   │\n" "$INIT_REL"
printf "  │ session.target:      T+%10sms                   │\n" "$SESSION_REL"
echo "  └────────────────────────────────────────────────────┘"

echo ""
echo "  参考指标："
echo "  • 正常范围：initialized.target < 1200ms"
echo "  • 优化目标：initialized.target < 1000ms"
echo "  • 优秀表现：initialized.target < 800ms"
echo ""

if [ "$INIT_REL" != "N/A" ] && [ "$INIT_REL" -gt 1200 ]; then
    echo -e "  ${RED}⚠  启动时间偏慢 (${INIT_REL}ms)，建议实施优化${NC}"
elif [ "$INIT_REL" != "N/A" ] && [ "$INIT_REL" -gt 1000 ]; then
    echo -e "  ${YELLOW}! 启动时间中等 (${INIT_REL}ms)，还有优化空间${NC}"
elif [ "$INIT_REL" != "N/A" ]; then
    echo -e "  ${GREEN}✓ 启动时间优秀 (${INIT_REL}ms)${NC}"
fi

echo ""
echo "═══════════════════════════════════════════════════════════"
echo "分析报告生成完毕"
echo "═══════════════════════════════════════════════════════════"

# 生成 Markdown 报告
generate_markdown_report() {
    local report_file="boot-analysis-$(date +%Y%m%d-%H%M%S).md"
    cat > "$report_file" << MDHEADER
# DDE 启动性能分析报告

> 生成时间：$(date '+%Y-%m-%d %H:%M:%S')

MDHEADER
    echo "" >> "$report_file"

    echo "## 1. systemd-analyze blame (Top 10)" >> "$report_file"
    echo '```' >> "$report_file"
    systemd-analyze --user blame 2>/dev/null | head -10 >> "$report_file"
    echo '```' >> "$report_file"
    echo "" >> "$report_file"

    echo "## 2. 关键时间线" >> "$report_file"
    echo "" >> "$report_file"
    echo "| Target | 时刻 (T+) |" >> "$report_file"
    echo "|--------|----------|" >> "$report_file"
    echo "| pre.target | ${PRE_REL}ms |" >> "$report_file"
    echo "| core.target | ${CORE_REL}ms |" >> "$report_file"
    echo "| initialized.target | ${INIT_REL}ms |" >> "$report_file"
    echo "| session.target | ${SESSION_REL}ms |" >> "$report_file"
    echo "" >> "$report_file"

    echo "## 3. 依赖关系检查" >> "$report_file"
    echo "" >> "$report_file"
    if [[ "$XSETTINGS_BEFORE" == *"Daemon1"* ]] || [[ "$DAEMON_BEFORE" == *"dde-session@x11"* ]]; then
        echo "- [ ] XSettings1 → Daemon1 → kwin 串行依赖 **待优化**" >> "$report_file"
    else
        echo "- [x] XSettings1/Daemon1/kwin 并行启动" >> "$report_file"
    fi
    echo "" >> "$report_file"

    echo "## 4. 优化建议" >> "$report_file"
    echo "" >> "$report_file"
    echo "1. 移除 XSettings1.Before=Daemon1" >> "$report_file"
    echo "2. 移除 Daemon1.Before=dde-session@x11" >> "$report_file"
    echo "3. dde-shell@DDE 改为 Type=notify" >> "$report_file"
    echo "4. kwin ExecStartPre 移出关键路径" >> "$report_file"
    echo "" >> "$report_file"

    echo "报告已保存到：$report_file"
}

# 如果传入 --report 参数，生成 Markdown 报告
if [ "$1" = "--report" ]; then
    generate_markdown_report
    exit 0
fi
