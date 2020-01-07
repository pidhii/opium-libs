#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "opium/opium.h"

#include <time.h>
#include <errno.h>
#include <string.h>

typedef struct Tm_s {
  OpiHeader header;
  opi_t sec, min, hour, mday, mon, year, wday, yday, isdst;
} Tm;

typedef struct TimeSpec_s {
  OpiHeader header;
  opi_t sec, nsec;
} TimeSpec;

static opi_type_t
tm_type, time_spec_type;

#define TM(x) ((Tm*)(x))
#define TIME_SPEC(x) ((TimeSpec*)(x))

static void
tm_delete(opi_type_t type, opi_t x)
{
  Tm *tm = TM(x);
  opi_unref(tm->sec);
  opi_unref(tm->min);
  opi_unref(tm->hour);
  opi_unref(tm->mday);
  opi_unref(tm->mon);
  opi_unref(tm->year);
  opi_unref(tm->wday);
  opi_unref(tm->yday);
  opi_unref(tm->isdst);
  free(tm);
}

static void
time_spec_delete(opi_type_t type, opi_t x)
{
  TimeSpec *ts = TIME_SPEC(x);
  opi_unref(ts->sec);
  opi_unref(ts->nsec);
  opi_h2w_free(ts);
}

static void
Tm_to_tm(opi_t src_, struct tm *dst)
{
  Tm *src = TM(src_);
  dst->tm_sec = OPI_NUM(src->sec)->val;
  dst->tm_min = OPI_NUM(src->min)->val;
  dst->tm_hour = OPI_NUM(src->hour)->val;
  dst->tm_mday = OPI_NUM(src->mday)->val;
  dst->tm_mon = OPI_NUM(src->mon)->val;
  dst->tm_year = OPI_NUM(src->year)->val;
  dst->tm_wday = OPI_NUM(src->wday)->val;
  dst->tm_yday = OPI_NUM(src->yday)->val;
  dst->tm_isdst = OPI_NUM(src->isdst)->val;
}

static void
TimeSpec_to_timespec(opi_t src_, struct timespec *dst)
{
  TimeSpec *src = TIME_SPEC(src_);
  dst->tv_sec = OPI_NUM(src->sec)->val;
  dst->tv_nsec = OPI_NUM(src->nsec)->val;
}

static opi_t
tm_new(const struct tm *tm)
{
  Tm *ret = malloc(sizeof(Tm));
  opi_inc_rc(ret->sec = opi_num_new(tm->tm_sec));
  opi_inc_rc(ret->min = opi_num_new(tm->tm_min));
  opi_inc_rc(ret->hour = opi_num_new(tm->tm_hour));
  opi_inc_rc(ret->mday = opi_num_new(tm->tm_mday));
  opi_inc_rc(ret->mon = opi_num_new(tm->tm_mon));
  opi_inc_rc(ret->year = opi_num_new(tm->tm_year));
  opi_inc_rc(ret->wday = opi_num_new(tm->tm_wday));
  opi_inc_rc(ret->yday = opi_num_new(tm->tm_yday));
  opi_inc_rc(ret->isdst = opi_num_new(tm->tm_isdst));
  opi_init_cell(ret, tm_type);
  return OPI(ret);
}

static opi_t
time_spec_new(const struct timespec *ts)
{
  TimeSpec *ret = opi_h2w();
  opi_inc_rc(ret->sec = opi_num_new(ts->tv_sec));
  opi_inc_rc(ret->nsec = opi_num_new(ts->tv_nsec));
  opi_init_cell(ret, time_spec_type);
  return OPI(ret);
}

static
OPI_DEF(time_, opi_return(opi_num_new(time(NULL))););

static
OPI_DEF(asctime_,
  opi_arg(x, tm_type)
  struct tm tm;
  Tm_to_tm(x, &tm);
  char *s = asctime(&tm);
  if (s == NULL)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(opi_str_new(s));
)

static
OPI_DEF(ctime_,
  opi_arg(t, opi_num_type)
  time_t tt = OPI_NUM(t)->val;
  char *s = ctime(&tt);
  if (s == NULL)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(opi_str_new(s));
)

static
OPI_DEF(getdate_,
  opi_arg(s, opi_str_type)
  struct tm *tm = getdate(OPI_STR(s)->str);
  if (tm == NULL)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(tm_new(tm));
)

static
OPI_DEF(gmtime_,
  opi_arg(t, opi_num_type)
  time_t tt = OPI_NUM(t)->val;
  struct tm *tm = gmtime(&tt);
  if (tm == NULL)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(tm_new(tm));
)

static
OPI_DEF(localtime_,
  opi_arg(t, opi_num_type)
  time_t tt = OPI_NUM(t)->val;
  struct tm *tm = localtime(&tt);
  if (tm == NULL)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(tm_new(tm));
)

static
OPI_DEF(mktime_,
  opi_arg(x, tm_type)
  struct tm tm;
  Tm_to_tm(x, &tm);
  time_t t = mktime(&tm);
  if (t == (time_t)-1)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(opi_num_new(t));
)

static
OPI_DEF(Clock_gettime,
  opi_arg(c, opi_num_type)
  struct timespec ts;
  if (clock_gettime(OPI_NUM(c)->val, &ts) < 0)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(time_spec_new(&ts));
)

static
OPI_DEF(TimeSpec_create,
  opi_arg(sec, opi_num_type)
  opi_arg(nsec, opi_num_type)
  if (OPI_NUM(nsec)->val > 1e9)
    opi_throw("domain-error");
  struct timespec ts;
  ts.tv_sec = OPI_NUM(sec)->val;
  ts.tv_nsec = OPI_NUM(nsec)->val;
  opi_return(time_spec_new(&ts));
)

static
OPI_DEF(Clock_getres,
  opi_arg(c, opi_num_type)
  struct timespec ts;
  if (clock_getres(OPI_NUM(c)->val, &ts) < 0)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(time_spec_new(&ts));
)

static
OPI_DEF(Clock_settime,
  opi_arg(c, opi_num_type)
  opi_arg(t, time_spec_type)
  struct timespec ts;
  TimeSpec_to_timespec(t, &ts);
  if (clock_settime(OPI_NUM(c)->val, &ts) < 0)
    opi_return(opi_undefined(opi_str_new(strerror(errno))));
  else
    opi_return(opi_nil);
)

int
opium_library(OpiBuilder *bldr)
{
  tm_type = opi_type_new("Tm");
  opi_type_set_delete_cell(tm_type, tm_delete);
  char *fields[] = { "sec", "min", "hour", "mday", "mon", "year", "wday", "yday", "isdst" };
  opi_type_set_fields(tm_type, offsetof(Tm, sec), fields, 9);
  opi_builder_def_type(bldr, "Tm", tm_type);

  time_spec_type = opi_type_new("TimeSpec");
  opi_type_set_delete_cell(time_spec_type, time_spec_delete);
  char *fields2[] = { "sec", "nsec" };
  opi_type_set_fields(time_spec_type, offsetof(TimeSpec, sec), fields2, 2);
  opi_builder_def_type(bldr, "TimeSpec", time_spec_type);
  opi_builder_def_const(bldr, "TimeSpec.create", opi_fn(0, TimeSpec_create, 2));

  opi_builder_def_const(bldr, "time", opi_fn(0, time_, 0));
  opi_builder_def_const(bldr, "asctime", opi_fn(0, asctime_, 1));
  opi_builder_def_const(bldr, "ctime", opi_fn(0, ctime_, 1));
  opi_builder_def_const(bldr, "getdate", opi_fn(0, getdate_, 1));
  opi_builder_def_const(bldr, "gmtime", opi_fn(0, gmtime_, 1));
  opi_builder_def_const(bldr, "localtime", opi_fn(0, localtime_, 1));

  opi_builder_def_const(bldr, "Clock.realtime", opi_num_new(CLOCK_REALTIME));
  opi_builder_def_const(bldr, "Clock.realtimeCoarse", opi_num_new(CLOCK_REALTIME_COARSE));
  opi_builder_def_const(bldr, "Clock.monotonic", opi_num_new(CLOCK_MONOTONIC));
  opi_builder_def_const(bldr, "Clock.monotonicCoarse", opi_num_new(CLOCK_MONOTONIC_COARSE));
  opi_builder_def_const(bldr, "Clock.monotonicRaw", opi_num_new(CLOCK_MONOTONIC_RAW));
  opi_builder_def_const(bldr, "Clock.boottime", opi_num_new(CLOCK_BOOTTIME));
  opi_builder_def_const(bldr, "Clock.processCputimeId", opi_num_new(CLOCK_PROCESS_CPUTIME_ID));
  opi_builder_def_const(bldr, "Clock.threadCputimeId", opi_num_new(CLOCK_THREAD_CPUTIME_ID));

  opi_builder_def_const(bldr, "Clock.gettime", opi_fn(0, Clock_gettime, 1));
  opi_builder_def_const(bldr, "Clock.getres", opi_fn(0, Clock_getres, 1));
  opi_builder_def_const(bldr, "Clock.settime", opi_fn(0, Clock_settime, 2));

  return 0;
}
