# procmon — A Tiny Process Monitor for Linux / WSL

A small `top`-like command-line tool written in C. It reads the Linux `/proc`
filesystem to list running processes, computes per-process CPU usage between two
samples, reports resident memory (RSS), and refreshes the screen periodically.

This is a learning project: the source code is heavily commented so you can see
exactly how Linux exposes process information.

## Features

- Enumerate all processes by scanning `/proc`
- Show PID, owner user, state, %CPU, resident memory (RSS), and command name
- Sort by CPU (default) or by memory (`-m`)
- Live refresh like `top`, or a single snapshot (`-1`)
- No external dependencies — only the C standard library and POSIX

## Build

```bash
make
```

This produces the `procmon` binary in the project directory.

## Usage

```bash
./procmon              # refresh every 2s, top 20 by CPU
./procmon -d 1         # refresh every 1 second
./procmon -n 15        # show only the top 15 processes
./procmon -m           # sort by memory instead of CPU
./procmon -1           # print one snapshot and exit
```

| Option | Meaning                                  |
| ------ | ---------------------------------------- |
| `-d N` | Refresh interval in seconds (default 2)  |
| `-n N` | Show the top N processes (default 20)    |
| `-m`   | Sort by memory usage instead of CPU      |
| `-1`   | Print a single snapshot and exit         |

Press `Ctrl-C` to quit the live view.

## How It Works

The Linux kernel exposes process information as virtual files under `/proc`:

| Source                  | Information used                                    |
| ----------------------- | -------------------------------------------------- |
| `/proc`                 | One numeric directory per process (the PID)        |
| `/proc/<pid>/stat`      | Command name, state, CPU time (`utime` + `stime`)  |
| `/proc/<pid>/status`    | `VmRSS` (resident memory), `Uid` (owner)           |
| `/proc/stat`            | Total CPU jiffies across all cores                 |

**CPU percentage** cannot be read directly. The kernel only reports *cumulative*
CPU time (in clock ticks, a.k.a. "jiffies"). So `procmon` takes two snapshots a
few seconds apart and computes:

```
%CPU = 100 * (process_jiffies_delta / total_jiffies_delta) * core_count
```

This is the same idea `top` and `htop` use.

## Project Layout

```
procmon/
├── Makefile          # build rules
├── README.md         # this file
└── src/
    └── procmon.c     # all of the source code, fully commented
```

## Notes for WSL

This works on WSL (WSL1 and WSL2) because WSL provides a real `/proc`
filesystem. On WSL2 the values reflect the Linux VM. Build and run from inside
your WSL shell, not from PowerShell.
