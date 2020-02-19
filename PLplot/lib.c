#include <opium/opium.h>
#include <plplot.h>

static
OPI_DEF(plparseopts_,
  opi_arg(args, opi_array_type)
  opi_arg(mode, opi_num_type)

  int argc = OPI_ARRAY(args)->len;
  char *argv[argc + 1];
  argv[argc] = NULL;
  for (int i = 0; i < argc; ++i) {
    opi_t x = OPI_ARRAY(args)->data[i];
    if (x->type != opi_str_type)
      opi_throw("type-error");
    argv[i] = OPI_STR(x)->str;
  }

  if (plparseopts(&argc, argv, OPI_NUM(mode)->val))
    opi_throw("plplot-error");

  opi_t ret = opi_nil;
  while (argc--)
    ret = opi_cons(opi_str_new(argv[argc]), ret);
  opi_return(ret);
)

OPI_DEF(plinit_, plinit();)
OPI_DEF(plend_, plend();)

OPI_DEF(plenv_,
  opi_arg(xmin, opi_num_type)
  opi_arg(xmax, opi_num_type)
  opi_arg(ymin, opi_num_type)
  opi_arg(ymax, opi_num_type)
  opi_arg(just, opi_num_type)
  opi_arg(axis, opi_num_type)
  plenv(OPI_NUM(xmin)->val, OPI_NUM(xmax)->val,
        OPI_NUM(ymin)->val, OPI_NUM(ymax)->val,
        OPI_NUM(just)->val, OPI_NUM(axis)->val);
)

OPI_DEF(pllab_,
  opi_arg(xlabel, opi_str_type)
  opi_arg(ylabel, opi_str_type)
  opi_arg(tlabel, opi_str_type)
  pllab(OPI_STR(xlabel)->str, OPI_STR(ylabel)->str, OPI_STR(tlabel)->str);
)

OPI_DEF(plline_,
  opi_arg(n, opi_num_type)
  opi_arg(x, opi_buffer_type)
  opi_arg(y, opi_buffer_type)
  plline(OPI_NUM(n)->val, OPI_BUFFER(x)->ptr, OPI_BUFFER(y)->ptr);
)

OPI_DEF(plcol0_,
  opi_arg(color, opi_num_type)
  plcol0(OPI_NUM(color)->val);
)

OPI_DEF(plclear_, plclear();)

OPI_DEF(plscol0_,
  opi_arg(icol0, opi_num_type)
  opi_arg(r, opi_num_type)
  opi_arg(g, opi_num_type)
  opi_arg(b, opi_num_type)
  plscol0(OPI_NUM(icol0)->val, OPI_NUM(r)->val, OPI_NUM(g)->val, OPI_NUM(b)->val);
)

OPI_DEF(plflush_, plflush();)

OPI_DEF(plxormod_,
  opi_arg(x, NULL)
  PLINT ret;
  plxormod(x != opi_false, &ret);
  opi_return(ret ? opi_true : opi_false);
)

int
opium_library(OpiBuilder *bldr)
{
  opi_builder_def_const(bldr, "_PL_PARSE_FULL", opi_num_new(PL_PARSE_FULL));
  opi_builder_def_const(bldr, "_PL_PARSE_QUIET", opi_num_new(PL_PARSE_QUIET));
  opi_builder_def_const(bldr, "_PL_PARSE_NODELETE", opi_num_new(PL_PARSE_NODELETE));
  opi_builder_def_const(bldr, "_PL_PARSE_SHOWALL", opi_num_new(PL_PARSE_SHOWALL));
  opi_builder_def_const(bldr, "_PL_PARSE_NOPROGRAM", opi_num_new(PL_PARSE_NOPROGRAM));
  opi_builder_def_const(bldr, "_PL_PARSE_NODASH", opi_num_new(PL_PARSE_NODASH));
  opi_builder_def_const(bldr, "_PL_PARSE_SKIP", opi_num_new(PL_PARSE_SKIP));
  opi_builder_def_const(bldr, "plparseopts", opi_fn_new(plparseopts_, 2));

  opi_builder_def_const(bldr, "plinit", opi_fn_new(plinit_, 0));
  opi_builder_def_const(bldr, "plend", opi_fn_new(plend_, 0));
  opi_builder_def_const(bldr, "plenv", opi_fn_new(plenv_, 6));
  opi_builder_def_const(bldr, "pllab", opi_fn_new(pllab_, 3));
  opi_builder_def_const(bldr, "plline", opi_fn_new(plline_, 3));
  opi_builder_def_const(bldr, "plcol0", opi_fn_new(plcol0_, 1));
  opi_builder_def_const(bldr, "plclear", opi_fn_new(plclear_, 0));
  opi_builder_def_const(bldr, "plscol0", opi_fn_new(plscol0_, 4));
  opi_builder_def_const(bldr, "plflush", opi_fn_new(plflush_, 0));
  opi_builder_def_const(bldr, "plxormod", opi_fn_new(plxormod_, 1));

  return 0;
}
