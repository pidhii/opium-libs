#include "opium/opium.h"

#include <getopt.h>

static
OPI_DEF(getopt_,
  opi_arg(s, opi_string_type);
  opi_arg(l, NULL)

  int n = opi_length(l);
  char *argv[n+1];

  for (int i = 0; i < n; l = opi_cdr(l), i++) {
    if (opi_car(l)->type != opi_string_type)
      opi_throw("type-error");
    argv[i+1] = OPI_STR(opi_car(l))->str;
  }

  int opt;
  optind = 1;
  opi_t acc = opi_nil;
  while ((opt = getopt(n + 1, argv, OPI_STR(s)->str)) > 0) {
    opi_t kv = opi_cons(opi_string_from_char(opt),
          optarg ? opi_string_new(optarg) : opi_nil);
    acc = opi_cons(kv , acc);
  }

  opi_return(acc);
)

int
opium_library(OpiBuilder *bldr)
{
  opi_builder_def_const(bldr, "getopt", opi_fn(0, getopt_, 2));
  return 0;
}
