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

#ifndef STL_PERF_H
#define STL_PERF_H

#include <linux/perf_event.h>
#include <stdint.h>

#define STL_PERF_VERSION "1.0.0"

#define STL_PERF_HW_CACHE(CACHE, OP, RESULT)                                    \
    ((PERF_COUNT_HW_CACHE_##CACHE) | (PERF_COUNT_HW_CACHE_OP_##OP << 8u) |     \
     (PERF_COUNT_HW_CACHE_RESULT_##RESULT << 16u))

typedef struct 
{
    char *name;
    uint64_t type;
    uint64_t config;
}stl_perf_event;

// clang-format off
static const  stl_perf_event stl_perf_hw[] = {
        {"cpu-clock",               PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK                  },
        {"task-clock",              PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK                 },
        {"page-faults",             PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS                },
        {"context-switches",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES           },
        {"cpu-migrations",          PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS             },
        {"page-fault-minor",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN            },
     // {"page-fault-major",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ            },
     // {"alignment-faults",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS           },
     // {"emulation-faults",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS           },

        {"cpu-cycles",              PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES                 },
        {"instructions",            PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS               },
     // {"cache-references",        PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES           },
        {"cache-misses",            PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES               },
     // {"branch-instructions",     PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS        },
     // {"branch-misses",           PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES              },
     // {"bus-cycles",              PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES                 },
     // {"stalled-cycles-frontend", PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND    },
     // {"stalled-cycles-backend",  PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND     },
     // {"ref-cpu-cycles",          PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES             },

     // {"L1D-read-access",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1D, READ, ACCESS)      },
        {"L1D-read-miss",           PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1D, READ, MISS)        },
     // {"L1D-write-access",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1D, WRITE, ACCESS)     },
     // {"L1D-write-miss",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1D, WRITE, MISS)       },
     // {"L1D-prefetch-access",     PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1D, PREFETCH, ACCESS)  },
     // {"L1D-prefetch-miss",       PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1D, PREFETCH, MISS)    },
     // {"L1I-read-access",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1I, READ, ACCESS)      },
        {"L1I-read-miss",           PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1I, READ, MISS)        },
     // {"L1I-write-access",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1I, WRITE, ACCESS)     },
     // {"L1I-write-miss",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1I, WRITE, MISS)       },
     // {"L1I-prefetch-access",     PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1I, PREFETCH, ACCESS)  },
     // {"L1I-prefetch-miss",       PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(L1I, PREFETCH, MISS)    },
     // {"LL-read-access",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(LL, READ, ACCESS)       },
     // {"LL-read-miss",            PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(LL, READ, MISS)         },
     // {"LL-write-access",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(LL, WRITE, ACCESS)      },
     // {"LL-write-miss",           PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(LL, WRITE, MISS)        },
     // {"LL-prefetch-access",      PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(LL, PREFETCH, ACCESS)   },
     // {"LL-prefetch-miss",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(LL, PREFETCH, MISS)     },
     // {"DTLB-read-access",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(DTLB, READ, ACCESS)     },
     // {"DTLB-read-miss",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(DTLB, READ, MISS)       },
     // {"DTLB-write-access",       PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(DTLB, WRITE, ACCESS)    },
     // {"DTLB-write-miss",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(DTLB, WRITE, MISS)      },
     // {"DTLB-prefetch-access",    PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(DTLB, PREFETCH, ACCESS) },
     // {"DTLB-prefetch-miss",      PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(DTLB, PREFETCH, MISS)   },
     // {"ITLB-read-access",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(ITLB, READ, ACCESS)     },
     // {"ITLB-read-miss",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(ITLB, READ, MISS)       },
     // {"ITLB-write-access",       PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(ITLB, WRITE, ACCESS)    },
     // {"ITLB-write-miss",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(ITLB, WRITE, MISS)      },
     // {"ITLB-prefetch-access",    PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(ITLB, PREFETCH, ACCESS) },
     // {"ITLB-prefetch-miss",      PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(ITLB, PREFETCH, MISS)   },
     // {"BPU-read-access",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(BPU, READ, ACCESS)      },
     // {"BPU-read-miss",           PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(BPU, READ, MISS)        },
     // {"BPU-write-access",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(BPU, WRITE, ACCESS)     },
     // {"BPU-write-miss",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(BPU, WRITE, MISS)       },
     // {"BPU-prefetch-access",     PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(BPU, PREFETCH, ACCESS)  },
     // {"BPU-prefetch-miss",       PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(BPU, PREFETCH, MISS)    },
     // {"NODE-read-access",        PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(NODE, READ, ACCESS)     },
     // {"NODE-read-miss",          PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(NODE, READ, MISS)       },
     // {"NODE-write-access",       PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(NODE, WRITE, ACCESS)    },
     // {"NODE-write-miss",         PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(NODE, WRITE, MISS)      },
     // {"NODE-prefetch-access",    PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(NODE, PREFETCH, ACCESS) },
     // {"NODE-prefetch-miss",      PERF_TYPE_HW_CACHE, STL_PERF_HW_CACHE(NODE, PREFETCH, MISS)   },
};

// clang-format on


void stl_perf_start();
void stl_perf_pause();
void stl_perf_end();

#endif
