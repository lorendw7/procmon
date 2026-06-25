/*
 * procmon - a tiny process monitor for Linux / WSL  [EXERCISE / FILL-IN VERSION]
 * -----------------------------------------------------------------------------
 * Your job: complete every block marked with  >>> TODO n <<<
 *
 * The scaffolding (struct, option parsing, printing, main loop) is given.
 * The interesting parts (reading /proc, parsing fields, computing %CPU) are
 * blanked out for you to write.
 *
 * Read EXERCISE.md for step-by-step hints in Chinese + English.
 * Reference solution lives in ../solution/procmon_solution.c (peek only if stuck).
 *
 * Build:  make
 * Run:    ./procmon -1 -n 8     (single snapshot, top 8 — good for testing)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <signal.h>

#define MAX_PROCS 4096

/* One process as we present it to the user. */
typedef struct {
    int   pid;
    char  comm[256];                /* command name                         */
    char  state;                    /* R, S, D, Z, T ...                    */
    char  user[64];                 /* owner user name                      */
    long  rss_kb;                   /* resident set size in KB              */
    unsigned long long cpu_jiffies; /* utime + stime at sample time         */
    double cpu_percent;             /* %CPU computed between two samples     */
} Proc;

static Proc prev[MAX_PROCS];
static int  prev_count = 0;

static volatile sig_atomic_t running = 1;
static void on_sigint(int sig) { (void)sig; running = 0; }

/* -------------------------------------------------------------------------
 * >>> TODO 1 <<<  Total CPU time across all cores.
 *
 * Open "/proc/stat", read the FIRST line which looks like:
 *     cpu  12345 67 8901 234567 ...
 * The first token is the literal word "cpu". Sum ALL the numbers after it
 * and return the total. Return 0 if the file cannot be opened.
 *
 * Hint: fscanf(f, "%15s", label) reads the word "cpu";
 *       then loop: while (fscanf(f, "%llu", &v) == 1) total += v;
 * ------------------------------------------------------------------------- */
static unsigned long long read_total_cpu_jiffies(void)
{
    /* TODO 1: replace the line below with your implementation */
    return 0;
}

/* Resolve a numeric uid into a user name (given — study it). */
static void uid_to_name(uid_t uid, char *out, size_t n)
{
    struct passwd *pw = getpwuid(uid);
    if (pw) snprintf(out, n, "%s", pw->pw_name);
    else    snprintf(out, n, "%u", uid);
}

/* -------------------------------------------------------------------------
 * Fill in one Proc from /proc/<pid>/stat and /proc/<pid>/status.
 * Returns 1 on success, 0 if the process vanished or could not be read.
 * ------------------------------------------------------------------------- */
static int read_proc(int pid, Proc *p)
{
    char path[64];
    char buf[1024];

    /* ---- /proc/<pid>/stat : read the whole single line into buf ---- */
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return 0; }
    fclose(f);

    /* ---------------------------------------------------------------------
     * >>> TODO 2 <<<  Extract the command name.
     *
     * The line looks like:  651 (yes) R 1 651 ...
     * The command name is wrapped in parentheses and MAY contain spaces or
     * even ')' itself, so:
     *   - find the FIRST '('   -> use strchr(buf, '(')
     *   - find the LAST  ')'   -> use strrchr(buf, ')')
     *   - the name is everything between them
     * Copy that text into p->comm (watch the buffer size!), null-terminate it.
     * If either parenthesis is missing, return 0.
     * --------------------------------------------------------------------- */
    char *lp = NULL, *rp = NULL;   /* TODO 2: set these and copy the name   */
    /* ... your code here ... */

    lp = strchr(buf, '(');
    rp = strrchr(buf, ')');
    if (!lp || !rp || lp >= rp) return 0;
    size_t len = rp - lp - 1;
    if (len > sizeof(p->comm) - 1) len = sizeof(p->comm) - 1;
    memcpy(p->comm, lp + 1, len);
    p->comm[len] = '\0';

    /* ---------------------------------------------------------------------
     * >>> TODO 3 <<<  Parse state + CPU time from the fields AFTER ')'.
     *
     * Counting fields as in proc(5) (1-indexed), the part after ')' starts at
     * field 3:
     *     field 3  = state   (a single char like 'R' or 'S')
     *     field 14 = utime   (CPU time in user mode,  in clock ticks)
     *     field 15 = stime   (CPU time in kernel mode, in clock ticks)
     *
     * Walk the tokens with strtok(rest, " ") / strtok(NULL, " "), keep a
     * field counter starting at 3, and capture state, utime, stime.
     * Then set:  p->state, p->cpu_jiffies = utime + stime.
     *
     * Hint: rest should point just past ") " (i.e. rp + 2).
     * --------------------------------------------------------------------- */
    p->pid = pid;
    char *rest = rp + 2;
    int field = 3;
    unsigned long long utime = 0, stime = 0;
    for (char *tok = strtok(rest, " "); tok != NULL; tok = strtok(NULL, " "), field++) {
        if (field == 3) {
            p->state = tok[0];               
        } else if (field == 14) {
            utime = strtoull(tok,
            NULL, 10);
        } else if (field == 15) {
            stime = strtoull(tok, NULL, 10);
            break;
        }
    }

    p->cpu_jiffies = utime + stime;            /* TODO 3: utime + stime                  */

    /* ---------------------------------------------------------------------
     * >>> TODO 4 <<<  Read memory + owner from /proc/<pid>/status.
     *
     * Open "/proc/<pid>/status" and read it line by line. Look for:
     *     "VmRSS:   1728 kB"   -> parse the number into p->rss_kb
     *     "Uid:     1000 ..."  -> parse first number, call uid_to_name()
     *
     * Hint: strncmp(buf, "VmRSS:", 6) == 0  to detect the line, then
     *       sscanf(buf + 6, "%ld", &p->rss_kb);
     * --------------------------------------------------------------------- */

     p->rss_kb = 0;
     p->user[0] = '\0';
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *sf = fopen(path, "r");
   if (sf)
   {
     while (fgets(buf, sizeof(buf), sf)) {
        if (strncmp(buf, "VmRSS:", 6) == 0) {
            sscanf(buf + 6, "%ld", &p->rss_kb);
        }
        if (strncmp(buf, "Uid:", 4) == 0)
        {
            int uid;
            sscanf(buf + 4, "%d", &uid);
            uid_to_name(uid, p->user, sizeof(p->user));
        }
        
    }
    fclose(sf);
   }
   

    if (p->user[0] == '\0') snprintf(p->user, sizeof(p->user), "?");
    p->cpu_percent = 0.0;
    return 1;
}

/* Look up a pid in the previous snapshot; returns index or -1 (given). */
static int find_prev(int pid)
{
    for (int i = 0; i < prev_count; i++)
        if (prev[i].pid == pid) return i;
    return -1;
}

/* qsort comparators: highest value first (given — study the trick). */
static int cmp_cpu(const void *a, const void *b)
{
    double da = ((const Proc *)a)->cpu_percent;
    double db = ((const Proc *)b)->cpu_percent;
    return (db > da) - (db < da);
}
static int cmp_mem(const void *a, const void *b)
{
    long da = ((const Proc *)a)->rss_kb;
    long db = ((const Proc *)b)->rss_kb;
    return (db > da) - (db < da);
}

int main(int argc, char **argv)
{
    int delay = 2, top_n = 20, by_mem = 0, once = 0;

    /* ---- option parsing (given) ---- */
    for (int i = 1; i < argc; i++) {
        if      (strcmp(argv[i], "-m") == 0) by_mem = 1;
        else if (strcmp(argv[i], "-1") == 0) once = 1;
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) delay = atoi(argv[++i]);
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) top_n = atoi(argv[++i]);
        else {
            fprintf(stderr,
                "Usage: %s [-d seconds] [-n count] [-m] [-1]\n", argv[0]);
            return 1;
        }
    }
    if (delay < 1) delay = 1;
    if (top_n < 1) top_n = 1;

    signal(SIGINT, on_sigint);

    long hz = sysconf(_SC_CLK_TCK);
    if (hz <= 0) hz = 100;

    Proc cur[MAX_PROCS];
    unsigned long long prev_total = read_total_cpu_jiffies();

    do {
        /* -----------------------------------------------------------------
         * >>> TODO 5 <<<  Scan /proc and build a fresh snapshot.
         *
         * - opendir("/proc")
         * - readdir() in a loop, skip entries whose name is NOT all-digits
         *   (hint: isdigit((unsigned char)de->d_name[0]); pid = atoi(name))
         * - for each pid, call read_proc(pid, &cur[count]); if it returns 1,
         *   increment count. Stop at MAX_PROCS.
         * - closedir()
         * ----------------------------------------------------------------- */
        int count = 0;
        /* ... your /proc scan here, filling cur[] and count ... */
        DIR *dir = opendir("/proc");
        if (!dir) break;
        struct dirent *de;
        while ((de = readdir(dir)) && count < MAX_PROCS) {
            if (!isdigit((unsigned char)de->d_name[0])) {
                continue;
            }

            int pid = atoi(de->d_name);
            if (read_proc(pid, &cur[count]) == 1)
            {
                count++;
            }
            
        }
        closedir(dir);
        /* ---- compute %CPU using delta against the previous sample ---- */
        unsigned long long total = read_total_cpu_jiffies();
        unsigned long long total_delta = total - prev_total;
        long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
        if (ncpu < 1) ncpu = 1;

        for (int i = 0; i < count; i++) {
            int j = find_prev(cur[i].pid);
            if (j >= 0 && total_delta > 0) {
                /* ---------------------------------------------------------
                 * >>> TODO 6 <<<  The core CPU formula.
                 *
                 * dj = how many jiffies THIS process used since last sample
                 *    = cur[i].cpu_jiffies - prev[j].cpu_jiffies
                 *
                 * cur[i].cpu_percent =
                 *     100.0 * (double)dj / (double)total_delta * ncpu;
                 * --------------------------------------------------------- */
                cur[i].cpu_percent = 0.0;   /* TODO 6: real formula */
            }
        }

        qsort(cur, count, sizeof(Proc), by_mem ? cmp_mem : cmp_cpu);

        /* ---- printing (given) ---- */
        if (!once) printf("\033[H\033[J");
        printf("procmon - %d processes - sorted by %s - HZ=%ld cores=%ld\n",
               count, by_mem ? "MEM" : "CPU", hz, ncpu);
        printf("%-8s %-12s %-2s %6s %12s  %s\n",
               "PID", "USER", "S", "%CPU", "RSS(KB)", "COMMAND");

        int shown = count < top_n ? count : top_n;
        for (int i = 0; i < shown; i++) {
            printf("%-8d %-12.12s %c  %6.1f %12ld  %s\n",
                   cur[i].pid, cur[i].user, cur[i].state,
                   cur[i].cpu_percent, cur[i].rss_kb, cur[i].comm);
        }
        fflush(stdout);

        memcpy(prev, cur, sizeof(Proc) * count);
        prev_count = count;
        prev_total = total;

        if (once) break;
        sleep(delay);
    } while (running);

    if (!once) printf("\nStopped.\n");
    return 0;
}
