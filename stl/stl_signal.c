/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
    #define _XOPEN_SOURCE 700
#endif

#include "stl_signal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
    #include <WinSock2.h>
volatile SOCKET stl_signal_shutdown_fd;
#else
volatile sig_atomic_t stl_signal_shutdown_fd;
#endif

/**
 * Set log file fd here, logging will be redirected to this fd, otherwise
 * STDERR_FILENO or STDOUT_FILENO will be used.
 */
volatile sig_atomic_t stl_signal_log_fd;

/**
 * Internal variable to handle twice shutdown signal.
 */
volatile sig_atomic_t stl_signal_will_shutdown;

#define get_uint(va, size)                                                     \
    (size) == 3 ? va_arg(va, unsigned long long) :                             \
    (size) == 2 ? va_arg(va, unsigned long) :                                  \
                  va_arg(va, unsigned int)

#define get_int(va, size)                                                      \
    (size) == 3 ? va_arg(va, long long) :                                      \
    (size) == 2 ? va_arg(va, long) :                                           \
                  va_arg(va, int)

#define PSIZE sizeof(void *) == sizeof(unsigned long long) ? 3 : 2

int stl_signal_vsnprintf(char *buf, size_t size, const char *fmt, va_list va)
{
    size_t len;
    size_t out_cap = size == 0 ? 0 : size - 1;
    char *str;
    char *out = buf;
    char *pos = (char *) fmt;
    char digits[32];

    while (true) {
        char *orig = pos;

        switch (*pos) {
        case '\0': {
            *out = '\0';
            return (int) (out - buf);
        }
        case '%': {
            pos++;
            switch (*pos) {
            case 's':
                str = (char *) va_arg(va, const char *);
                str = (str == NULL) ? "(null)" : str;
                len = strlen(str);
                break;
            case 'l':
                pos += (*(pos + 1) == 'l') ? 2 : 1; // fall through
            case 'd':
            case 'u':
                len = 0;

                if (*pos == 'u') {
                    uint64_t u64 = get_uint(va, pos - orig);

                    do {
                        digits[31 - (len++)] = (char) ('0' + (u64 % 10));
                    } while (u64 /= 10UL);

                } else if (*pos == 'd') {
                    int64_t i64 = get_int(va, pos - orig);
                    uint64_t u64 = (uint64_t)(i64 < 0 ? -i64 : i64);

                    do {
                        digits[31 - (len++)] = (char) ('0' + (char) (u64 % 10));
                    } while (u64 /= 10UL);

                    if (i64 < 0) {
                        digits[31 - (len++)] = '-';
                    }
                } else {
                    return -1;
                }

                str = &digits[32 - len];
                break;
            case 'p': {
                const char *arr = "0123456789abcdef";
                len = 0;
                uint64_t u64 = get_uint(va, PSIZE);

                do {
                    digits[31 - (len++)] = arr[u64 % 16];
                } while (u64 /= 16UL);

                digits[31 - (len++)] = 'x';
                digits[31 - (len++)] = '0';
                str = &digits[32 - len];
            } break;
            case '%':
                str = "%";
                len = 1;
                break;
            default:
                return -1;
            }
            pos++;
        } break;
        default: {
            while (*pos != '\0' && *pos != '%') {
                pos++;
            }
            str = orig;
            len = pos - orig;
            break;
        }
        }

        len = len < out_cap ? len : out_cap;
        memcpy(out, str, len);

        out += len;
        out_cap -= len;
    }
}

int stl_signal_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = stl_signal_vsnprintf(buf, size, fmt, args);
    va_end(args);

    return ret;
}

#if defined(_WIN32)

    #define WIN32_LEAN_AND_MEAN

    #include <Ws2tcpip.h>
    #include <io.h>
    #include <signal.h>
    #include <windows.h>

    #pragma warning(disable : 4996)
    #pragma comment(lib, "Ws2_32.lib")

BOOL WINAPI stl_console_handler(DWORD type)
{
    int rc;
    char *err;
    char buf[128];
    int fd = stl_signal_log_fd != -1 ? stl_signal_log_fd : _fileno(stdout);

    switch (type) {
    case CTRL_C_EVENT:
        err = "CTRL_C event";
        break;
    case CTRL_BREAK_EVENT:
        err = "CTRL_BREAK event";
        break;
    default:
        stl_signal_log(fd, buf, sizeof(buf),
                      "Unknown console event [%d], shutting down! \n", type);
        _Exit(1);
    }

    stl_signal_log(fd, buf, sizeof(buf), "Received : %s, (%d) \n", err, type);

    if (stl_signal_will_shutdown != 0) {
        stl_signal_log(fd, buf, sizeof(buf), "Forcing shut down! \n");
        _Exit(1);
    }

    stl_signal_will_shutdown = 1;

    if (stl_signal_shutdown_fd != INVALID_SOCKET) {
        stl_signal_log(fd, buf, sizeof(buf), "Sending shutdown command. \n");
        rc = send(stl_signal_shutdown_fd, (void *) &(int){1}, 1, 0);
        if (rc != 1) {
            stl_signal_log(fd, buf, sizeof(buf),
                          "Failed to send shutdown command, "
                          "shutting down immediately! \n");
            _Exit(1);
        }
    } else {
        stl_signal_log(fd, buf, sizeof(buf),
                      "No shutdown handler, shutting down! \n");
        _Exit(0);
    }


    return TRUE;
}

LONG WINAPI stl_signal_on_fatal(PEXCEPTION_POINTERS info)
{
    char buf[128];
    int fd = stl_signal_log_fd != -1 ? stl_signal_log_fd : _fileno(stderr);

    stl_signal_log(fd, buf, sizeof(buf), "Fatal signal : %d, shutting down! \n",
                  info->ExceptionRecord->ExceptionCode);

    return 0;
}

void stl_signal_std_on_fatal(int sig)
{
    char buf[128];
    int fd = stl_signal_log_fd != -1 ? stl_signal_log_fd : _fileno(stderr);
    char *sig_str = "unknown signal";

    if (sig == SIGSEGV) {
        sig_str = "SIGSEGV";
    } else if (sig == SIGABRT) {
        sig_str = "SIGABRT";
    } else if (sig == SIGFPE) {
        sig_str = "SIGFPE";
    } else if (sig == SIGILL) {
        sig_str = "SIGILL";
    }

    stl_signal_log(fd, buf, sizeof(buf),
                  "Fatal signal : [%s][%d], shutting down! \n", sig_str, sig);

    _Exit(1);
}

void stl_signal_std_on_shutdown(int type)
{
    stl_console_handler(CTRL_C_EVENT);
}
int stl_signal_init()
{
    BOOL b;
    stl_signal_log_fd = -1;
    stl_signal_shutdown_fd = -1;

    b = SetConsoleCtrlHandler(stl_console_handler, TRUE);
    if (!b) {
        return -1;
    }

    SetUnhandledExceptionFilter(stl_signal_on_fatal);
    signal(SIGABRT, stl_signal_std_on_fatal);
    signal(SIGINT, stl_signal_std_on_shutdown);

    return 0;
}

#else

    // clang-format off
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>

static void *stl_instruction(ucontext_t *uc)
{
    void* p = NULL;

#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)
    #if defined(_STRUCT_X86_THREAD_STATE64) && !defined(__i386__)
            p = (void *) uc->uc_mcontext->__ss.__rip;
        #elif defined(__i386__)
            p = (void *) uc->uc_mcontext->__ss.__eip;
        #else
            p = (void *) arm_thread_state64_get_pc(uc->uc_mcontext->__ss);
        #endif
#elif defined(__linux__)
#if defined(__i386__) || ((defined(__x86_64__)) && defined(__ILP32__))
    p = (void *) uc->uc_mcontext.gregs[REG_EIP];
#elif defined(__x86_64__)
    p = (void *) uc->uc_mcontext.gregs[REG_RIP];
#elif defined(__ia64__)
    p = (void *) uc->uc_mcontext.stl_ip;
        #elif defined(__arm__)
            p = (void *) uc->uc_mcontext.arm_pc;
        #elif defined(__aarch64__)
            p = (void *) uc->uc_mcontext.pc;
#endif
#elif defined(__FreeBSD__)
    #if defined(__i386__)
            p = (void *) uc->uc_mcontext.mc_eip;
        #elif defined(__x86_64__)
            p = (void *) uc->uc_mcontext.mc_rip;
        #endif
    #elif defined(__OpenBSD__)
        #if defined(__i386__)
            p = (void *) uc->stl_eip;
        #elif defined(__x86_64__)
            p = (void *) uc->stl_rip;
        #endif
    #elif defined(__NetBSD__)
        #if defined(__i386__)
            p = (void *) uc->uc_mcontext.__gregs[_REG_EIP];
        #elif defined(__x86_64__)
            p = (void *) uc->uc_mcontext.__gregs[_REG_RIP];
        #endif
    #elif defined(__DragonFly__)
            p = (void *) uc->uc_mcontext.mc_rip;
#endif

    return p;
}

#endif

// clang-format on

static void stl_signal_on_shutdown(int sig)
{
    int rc;
    int saved_errno = errno;
    int fd = stl_signal_log_fd != -1 ? stl_signal_log_fd : STDOUT_FILENO;
    char buf[4096], *sig_str = "Shutdown signal";

    if (sig == SIGINT) {
        sig_str = "SIGINT";
    } else if (sig == SIGTERM) {
        sig_str = "SIGTERM";
    }

    stl_signal_log(fd, buf, sizeof(buf), "Received : %s, (%d) \n", sig_str, sig);

    if (stl_signal_will_shutdown != 0) {
        stl_signal_log(fd, buf, sizeof(buf), "Forcing shut down! \n");
    #ifdef stl_SIGNAL_TEST
        return;
    #endif
        _Exit(1);
    }

    stl_signal_will_shutdown = 1;

    if (stl_signal_shutdown_fd != -1) {
        stl_signal_log(fd, buf, sizeof(buf), "Sending shutdown command. \n");
        rc = (int) write(stl_signal_shutdown_fd, (void *) &(int){1}, 1);
        if (rc != 1) {
            stl_signal_log(fd, buf, sizeof(buf),
                          "Failed to send shutdown command, "
                          "shutting down immediately! \n");
    #ifdef stl_SIGNAL_TEST
            return;
    #endif
            _Exit(1);
        }
    } else {
        stl_signal_log(fd, buf, sizeof(buf),
                      "No shutdown handler, shutting down! \n");
    #ifdef stl_SIGNAL_TEST
        return;
    #endif
        _Exit(0);
    }

    errno = saved_errno;
}

static void stl_signal_on_fatal(int sig, siginfo_t *info, void *context)
{
    (void) info;

    int fd = stl_signal_log_fd != -1 ? stl_signal_log_fd : STDERR_FILENO;

    char buf[4096], *sig_str = "unknown signal";
    struct sigaction act;

    if (sig == SIGSEGV) {
        sig_str = "SIGSEGV";
    } else if (sig == SIGABRT) {
        sig_str = "SIGABRT";
    } else if (sig == SIGBUS) {
        sig_str = "SIGBUS";
    } else if (sig == SIGFPE) {
        sig_str = "SIGFPE";
    } else if (sig == SIGILL) {
        sig_str = "SIGILL";
    }

    stl_signal_log(fd, buf, sizeof(buf), "\nSignal received : [%d][%s] \n", sig,
                  sig_str);

    stl_signal_log(fd, buf, sizeof(buf),
                  "\n----------------- CRASH REPORT ---------------- \n");

#ifdef HAVE_BACKTRACE
    void *caller = stl_instruction((ucontext_t *) context);
    int trace_size;
    void *trace[100];

    stl_signal_log(fd, buf, sizeof(buf), "\n Caller [%p] \n\n", caller);

    trace_size = backtrace(trace, 100);
    backtrace_symbols_fd(trace, trace_size, fd);
#else
    (void) context;
#endif
    stl_signal_log(fd, buf, sizeof(buf),
                  "\n--------------- CRASH REPORT END -------------- \n");

    stl_signal_log(fd, buf, sizeof(buf), "\nSignal handler completed! \n");
    close(fd);

    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
    act.sa_handler = SIG_DFL;
    sigaction(sig, &act, NULL);

#ifndef stl_SIGNAL_TEST
    kill(getpid(), sig);
#endif
}

int stl_signal_init()
{
    bool rc = true;
    struct sigaction action;

    stl_signal_log_fd = -1;
    stl_signal_shutdown_fd = -1;

    rc &= (signal(SIGHUP, SIG_IGN) != SIG_ERR);
    rc &= (signal(SIGPIPE, SIG_IGN) != SIG_ERR);
    rc &= (sigemptyset(&action.sa_mask) == 0);

    action.sa_flags = 0;
    action.sa_handler = stl_signal_on_shutdown;

    rc &= (sigaction(SIGTERM, &action, NULL) == 0);
    rc &= (sigaction(SIGINT, &action, NULL) == 0);

#ifdef stl_SIGNAL_TEST
    rc &= (sigaction(SIGUSR2, &action, NULL) == 0);
#endif

    rc &= (sigemptyset(&action.sa_mask) == 0);
    action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
    action.sa_sigaction = stl_signal_on_fatal;

    rc &= (sigaction(SIGABRT, &action, NULL) == 0);
    rc &= (sigaction(SIGSEGV, &action, NULL) == 0);
    rc &= (sigaction(SIGBUS, &action, NULL) == 0);
    rc &= (sigaction(SIGFPE, &action, NULL) == 0);
    rc &= (sigaction(SIGILL, &action, NULL) == 0);

#ifdef stl_SIGNAL_TEST
    rc &= (sigaction(SIGSYS, &action, NULL) == 0);
#endif

    return rc ? 0 : -1;
}

#endif

void stl_signal_log(int fd, char *buf, size_t len, char *fmt, ...)
{
    int written, rc;
    va_list args;

    va_start(args, fmt);
    written = stl_signal_vsnprintf(buf, len, fmt, args);
    va_end(args);

    rc = write(fd, buf, (size_t) written);
    (void) rc;
}
