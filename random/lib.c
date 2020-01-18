#include <opium/opium.h>
#include <stdlib.h>
#include <string.h>

#define is_rand_buf(x) \
  (x->type == opi_buffer_type && \
   OPI_BUFFER(x)->size >= sizeof(int))

static
OPI_DEF(srand_,
  opi_arg(seed, opi_num_type)
  srand(OPI_NUM(seed)->val);
)

static opi_t
rand_(void)
{
  return opi_num_new(rand());
}

static opi_t
rand_r_(void)
{
  opi_t buf = opi_pop();
  if (opi_unlikely(!is_rand_buf(buf))) {
    opi_drop(buf);
    return opi_undefined(opi_symbol("buffer-size-error"));
  }
  return opi_num_new(rand_r(OPI_BUFFER(buf)->ptr));
}

/*******************************************************************************
 *                            Congruental generators
 ******************************************************************************/
typedef struct {
  OpiHeader header;
  struct drand48_data data;
} DRand48Data;
#define DRAND48_DATA(x) ((DRand48Data*)(x))

static
opi_type_t drand48_data_type;

static opi_t
drand48_data_new(struct drand48_data *data)
{
  DRand48Data *d48 = malloc(sizeof(DRand48Data));
  memcpy(&d48->data, data, sizeof(struct drand48_data));
  opi_init_cell(d48, drand48_data_type);
  return OPI(d48);
}

static opi_t
DRand48Data_copy(void)
{
  opi_t d48 = opi_pop();
  if (opi_unlikely(d48->type != drand48_data_type)) {
    opi_drop(d48);
    return opi_undefined(opi_symbol("type-error"));
  }
  if (d48->rc == 0)
    return d48;
  else
    return drand48_data_new(&DRAND48_DATA(d48)->data);
}

static
OPI_DEF(srand48_,
  opi_arg(seed, opi_num_type)
  srand48(OPI_NUM(seed)->val);
)

static
OPI_DEF(seed48_,
  opi_arg(seed, opi_buffer_type)
  if (OPI_BUFFER(seed)->size < sizeof(short[3]))
    opi_throw("buffer-size-error");
  unsigned short *old = seed48(OPI_BUFFER(seed)->ptr);
  unsigned short *buf = malloc(sizeof(short[3]));
  memcpy(buf, old, sizeof(short[3]));
  opi_return(OPI(opi_buffer_new(buf, sizeof(short[3]), OPI_BUFFER_FREE, NULL)));
)

static
OPI_DEF(lcong48_,
  opi_arg(seed, opi_buffer_type)
  if (OPI_BUFFER(seed)->size < sizeof(short[7]))
    opi_throw("buffer-size-error");
  lcong48(OPI_BUFFER(seed)->ptr);
)

static opi_t
drand48_(void)
{
  return opi_num_new(drand48());
}

static opi_t
lrand48_(void)
{
  return opi_num_new(lrand48());
}

static opi_t
mrand48_(void)
{
  return opi_num_new(mrand48());
}

static
OPI_DEF(srand48_r_,
  opi_arg(seed, opi_num_type)
  struct drand48_data data;
  srand48_r(OPI_NUM(seed)->val, &data);
  opi_drop(seed);
  opi_return(drand48_data_new(&data));
)

static
OPI_DEF(seed48_r_,
  opi_arg(seed, opi_buffer_type)
  if (OPI_BUFFER(seed)->size < sizeof(short[3]))
    opi_throw("buffer-size-error");
  struct drand48_data data;
  seed48_r(OPI_BUFFER(seed)->ptr, &data);
  opi_return(drand48_data_new(&data));
)

static
OPI_DEF(lcong48_r_,
  opi_arg(seed, opi_buffer_type)
  if (OPI_BUFFER(seed)->size < sizeof(short[7]))
    opi_throw("buffer-size-error");
  struct drand48_data data;
  lcong48_r(OPI_BUFFER(seed)->ptr, &data);
  opi_return(drand48_data_new(&data));
)

static opi_t
drand48_r_(void)
{
  opi_t buf = opi_pop();
  if (opi_unlikely(buf->type != drand48_data_type)) {
    opi_drop(buf);
    return opi_undefined(opi_symbol("type-error"));
  }
  double ret;
  drand48_r(&DRAND48_DATA(buf)->data, &ret);
  opi_drop(buf);
  return opi_num_new(ret);
}

static opi_t
lrand48_r_(void)
{
  opi_t buf = opi_pop();
  if (opi_unlikely(buf->type != drand48_data_type)) {
    opi_drop(buf);
    return opi_undefined(opi_symbol("type-error"));
  }
  long int ret;
  lrand48_r(&DRAND48_DATA(buf)->data, &ret);
  opi_drop(buf);
  return opi_num_new(ret);
}

static opi_t
mrand48_r_(void)
{
  opi_t buf = opi_pop();
  if (opi_unlikely(buf->type != drand48_data_type)) {
    opi_drop(buf);
    return opi_undefined(opi_symbol("type-error"));
  }
  long int ret;
  mrand48_r(&DRAND48_DATA(buf)->data, &ret);
  opi_drop(buf);
  return opi_num_new(ret);
}

/*******************************************************************************
 *                            Nonlinear additive ...
 ******************************************************************************/
static
OPI_DEF(srandom_,
  opi_arg(seed, opi_num_type)
  srandom(OPI_NUM(seed)->val);
)

static opi_t
random_(void)
{
  return opi_num_new(random());
}

int
opium_library(OpiBuilder *bldr)
{
  drand48_data_type = opi_type_new("DRand48Data");
  opi_type_set_delete_cell(drand48_data_type, OPI_FREE_CELL);
  opi_builder_def_type(bldr, "DRand48Data", drand48_data_type);

  opi_builder_def_const(bldr, "randMax", opi_num_new(RAND_MAX));

  opi_builder_def_const(bldr, "srand", opi_fn_new(srand_, 1));
  opi_builder_def_const(bldr, "rand", opi_fn_new(rand_, 0));
  opi_builder_def_const(bldr, "rand_r", opi_fn_new(rand_r_, 1));

  opi_builder_def_const(bldr, "DRand48Data.copy", opi_fn_new(DRand48Data_copy, 1));
  opi_builder_def_const(bldr, "srand48", opi_fn_new(srand48_, 1));
  opi_builder_def_const(bldr, "srand48_r", opi_fn_new(srand48_r_, 1));
  opi_builder_def_const(bldr, "seed48", opi_fn_new(seed48_, 1));
  opi_builder_def_const(bldr, "seed48_r", opi_fn_new(seed48_r_, 1));
  opi_builder_def_const(bldr, "lcong48", opi_fn_new(lcong48_, 1));
  opi_builder_def_const(bldr, "lcong48_r", opi_fn_new(lcong48_r_, 1));
  opi_builder_def_const(bldr, "drand48", opi_fn_new(drand48_, 0));
  opi_builder_def_const(bldr, "drand48_r", opi_fn_new(drand48_r_, 1));
  opi_builder_def_const(bldr, "lrand48", opi_fn_new(lrand48_, 0));
  opi_builder_def_const(bldr, "lrand48_r", opi_fn_new(lrand48_r_, 1));
  opi_builder_def_const(bldr, "mrand48", opi_fn_new(mrand48_, 0));
  opi_builder_def_const(bldr, "mrand48_r", opi_fn_new(mrand48_r_, 1));

  opi_builder_def_const(bldr, "srandom", opi_fn_new(srandom_, 1));
  opi_builder_def_const(bldr, "random", opi_fn_new(random_, 0));
  return 0;
}
