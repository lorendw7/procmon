# procmon — Exercise Guide (练习指南)

The file `src/procmon.c` is a **fill-in-the-blank** skeleton. Complete the six
`>>> TODO n <<<` blocks yourself. This guide gives the goal of each blank in
English plus a Chinese hint. The full answer is in
`solution/procmon_solution.c` — only peek when truly stuck.

`src/procmon.c` 是挖空版,请自己完成 6 个 `TODO`。下面每个 TODO 配中英文提示。
答案在 `solution/procmon_solution.c`,实在卡住再看。

---

## Recommended order (建议顺序)

Do them in this order so you can test as you go:
**TODO 5 → 2 → 3 → 4 → 1 → 6**

Why: once TODO 5 (scan /proc) + TODO 2/3/4 (read one process) work, run
`./procmon -1` and you already see a process list. TODO 1 + 6 add the %CPU.

先做能"看到东西"的部分,再补 CPU 百分比,方便边写边验证。

---

## TODO 1 — `read_total_cpu_jiffies()`
**Goal:** sum all numbers on the first line of `/proc/stat`.
**目标:** 把 `/proc/stat` 第一行(以 `cpu` 开头)后面所有数字加起来,得到全系统 CPU 总时间。

- Open the file, read the word `"cpu"` first, then loop summing `%llu` values.
- 先 `fopen`,用 `fscanf(f, "%15s", label)` 吃掉 "cpu",再 `while (fscanf(f,"%llu",&v)==1) total+=v;`
- Try it: `cat /proc/stat | head -1` 看看真实数据长什么样。

## TODO 2 — command name (命令名)
**Goal:** copy the text between the first `(` and the last `)` into `p->comm`.
**目标:** 把 `/proc/<pid>/stat` 里括号中间的命令名拷进 `p->comm`。

- `strchr(buf,'(')` finds the first `(`; `strrchr(buf,')')` finds the LAST `)`.
- ⚠️ 命令名可能含空格甚至 `)`,所以必须用**最后一个** `)`,不能直接按空格切。
- Length = `rp - lp - 1`. Clamp to `sizeof(p->comm)-1`, then null-terminate.
- If a parenthesis is missing, `return 0`.

## TODO 3 — state + CPU jiffies (状态 + CPU 时间)
**Goal:** from the fields after `") "`, grab field 3 (state), 14 (utime), 15 (stime).
**目标:** 解析 `)` 之后的字段:第 3 个是状态,第 14 个 utime,第 15 个 stime。

- `char *rest = rp + 2;` then `strtok(rest, " ")` / `strtok(NULL, " ")`.
- Keep a counter starting at **3**. Save state at 3, utime at 14, stime at 15.
- Set `p->state` and `p->cpu_jiffies = utime + stime;`
- 字段编号参考 `man 5 proc` 里的 `/proc/[pid]/stat` 一节。

## TODO 4 — memory + owner (内存 + 属主)
**Goal:** read `/proc/<pid>/status`, fill `p->rss_kb` (VmRSS) and `p->user` (Uid).
**目标:** 逐行读 `status`,找 `VmRSS:` 取内存,找 `Uid:` 取 uid 再转用户名。

- `while (fgets(buf,...,f))` then `strncmp(buf,"VmRSS:",6)==0` → `sscanf(buf+6,"%ld",&p->rss_kb)`.
- For `"Uid:"`: parse the first number, call `uid_to_name(uid, p->user, sizeof(p->user))`.
- 看一眼真实数据:`cat /proc/self/status | grep -E 'VmRSS|Uid'`

## TODO 5 — scan /proc (枚举进程)
**Goal:** loop over `/proc`, keep all-digit names as PIDs, call `read_proc`.
**目标:** 遍历 `/proc`,只保留纯数字目录名(就是 PID),逐个 `read_proc`。

- `DIR *d = opendir("/proc");` then `while ((de = readdir(d)) && count < MAX_PROCS)`.
- Skip if `!isdigit((unsigned char)de->d_name[0])`. `pid = atoi(de->d_name);`
- If `read_proc(pid, &cur[count])` returns 1, do `count++`. Then `closedir(d)`.

## TODO 6 — the %CPU formula (核心公式)
**Goal:** scale this process's jiffy delta by the system's total delta.
**目标:** 用本进程的时间增量 ÷ 全系统时间增量,乘 100 和核心数。

```c
unsigned long long dj = cur[i].cpu_jiffies - prev[j].cpu_jiffies;
cur[i].cpu_percent = 100.0 * (double)dj / (double)total_delta * ncpu;
```
- 第一帧一定是 0%(没有上一帧可减),这是正常的。

---

## Build & test (编译与测试)

```bash
make                       # 编译;有报错就按提示改
./procmon -1 -n 8          # 单帧测试:先验证进程列表对不对
./procmon -d 1             # 实时模式:制造负载看 %CPU
```

Make some CPU load in another terminal to see %CPU move:
```bash
yes > /dev/null            # 占满一个核;测完按 Ctrl-C 结束
```

## Self-check (自查清单)
- [ ] `make` 0 warnings (Makefile 开了 `-Wall -Wextra`)
- [ ] PID / USER / COMMAND 显示正确
- [ ] RSS(KB) 是合理的数字,不是 0
- [ ] 实时模式下 `yes` 进程能冲到 ~100% CPU
- [ ] `Ctrl-C` 能干净退出
