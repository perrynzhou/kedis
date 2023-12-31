# Makefile.am - automake file for lcdb
# Copyright (c) 2022, Christopher Jeffrey (MIT License).
# https://github.com/chjj/lcdb

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
AM_LDFLAGS = -static
LDADD = libtestutil.la $(top_builddir)/liblcdb.la

libtestutil_la_SOURCES = $(top_srcdir)/src/util/testutil.c \
                         $(top_srcdir)/src/util/testutil.h

noinst_HEADERS = data/snappy_data.h tests.h

check_LTLIBRARIES = libtestutil.la

check_PROGRAMS = t-arena        \
                 t-autocompact  \
                 t-bloom        \
                 t-c            \
                 t-cache        \
                 t-coding       \
                 t-corruption   \
                 t-crc32c       \
                 t-db           \
                 t-dbformat     \
                 t-env          \
                 t-filename     \
                 t-filter_block \
                 t-hash         \
                 t-issue178     \
                 t-issue200     \
                 t-issue320     \
                 t-log          \
                 t-rbt          \
                 t-recovery     \
                 t-simple       \
                 t-skiplist     \
                 t-snappy       \
                 t-status       \
                 t-strutil      \
                 t-table        \
                 t-util         \
                 t-version_edit \
                 t-version_set  \
                 t-write_batch

TESTS = $(check_PROGRAMS)

if ENABLE_TESTS
all-local: $(check_PROGRAMS)
endif
