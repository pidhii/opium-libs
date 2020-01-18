#include "opium/opium.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>

static int
read_sexpr(FILE *in, opi_t *expr);

static int
skipws(FILE *in)
{
  int ch;
  while (isspace(ch = getc(in)));
  if (ch != EOF)
    ungetc(ch, in);
  return ch;
}

static int
peek(FILE *in)
{
  char ch = getc(in);
  if (ch != EOF)
    ungetc(ch, in);
  return ch;
}


static opi_t
read_list(FILE *in)
{
  getc(in); // ignore leading bracket

  cod_vec(opi_t) buf;
  cod_vec_init(buf);
  while (TRUE) {
    skipws(in);
    char c = peek(in);

    if (c == ')') { // reached closing bracket
      getc(in); // get the bracket
      break;
    }

    if (c == EOF) {
      opi_error("unexpected end of input, expecting ')'\n");
      goto error;
    }

    opi_t e;
    if (read_sexpr(in, &e) < 0 && opi_error)
      goto error;

    cod_vec_push(buf, e);
  }

  opi_t l = opi_nil;
  while (buf.len)
    l = opi_cons(cod_vec_pop(buf), l);
  cod_vec_destroy(buf);
  return l;

error:
  while (buf.len)
    opi_drop(cod_vec_pop(buf));
  cod_vec_destroy(buf);
  opi_error = 1;
  return NULL;
}

static opi_t
read_quote(FILE *in)
{
  static opi_t quote = NULL;
  if (quote == NULL)
    quote = opi_symbol("quote");

  getc(in); // skip leading single quote
  opi_t x;
  if (read_sexpr(in, &x) < 0) {
    if (opi_error) // parse-error
      return NULL;
    // end of file
    opi_error("unexpected end of input, expected expression after quote\n");
    opi_error = 1;
    return NULL;
  }
  return opi_cons(quote, opi_cons(x, opi_nil));
}

static int
issym(int c)
{
  return !isspace(c) &&
         !iscntrl(c) &&
         c != EOF &&
         c != '(' && c != ')'
  ;
}

static opi_t
read_atom(FILE *in)
{
  cod_vec(char) buf;
  cod_vec_init(buf);

  char c;
  while (issym(c = getc(in)))
    cod_vec_push(buf, c);
  if (c != EOF)
    ungetc(c, in);

  cod_vec_push(buf, 0);
  char *end;
  long double num = strtold(buf.data, &end);
  if (*end == 0) {
    // this is a number
    cod_vec_destroy(buf);
    return opi_num_new(num);
  } else {
    // this is a symbol
    opi_t sym = opi_symbol(buf.data);
    cod_vec_destroy(buf);
    return sym;
  }
}

static int
read_sexpr(FILE *in, opi_t *expr)
{
  opi_error = 0;

  if (skipws(in) == EOF)
    return -1;

  int ch = peek(in);
  opi_t ret;
  if (ch == '(')
    ret = read_list(in);
  else if (ch == '\'')
    ret = read_quote(in);
  else
    ret = read_atom(in);

  if (ret == NULL)
    return -1;
  *expr = ret;
  return 0;
}


OPI_DEF(read_,
  opi_arg(file, opi_file_type)
  opi_t ret;
  opi_error = 0;
  if (read_sexpr(opi_file_get_value(file), &ret) < 0) {
    if (opi_error)
      opi_throw("parse-error");
    opi_throw("empty-input");
  }
  opi_return(ret);
)

OPI_DEF(parse_,
  opi_arg(buf, opi_buffer_type)
  FILE *file = fmemopen(OPI_BUFFER(buf)->ptr, OPI_BUFFER(buf)->size, "r");
  if (file == NULL)
    opi_throw(strerror(errno));
  opi_t ret;
  opi_error = 0;
  if (read_sexpr(file, &ret) < 0) {
    fclose(file);
    if (opi_error)
      opi_throw("parse-error");
    opi_throw("empty-input");
  }
  fclose(file);
  opi_return(ret);
)

int
opium_library(OpiBuilder *bldr)
{
  opi_builder_def_const(bldr, "read", opi_fn_new(read_, 1));
  opi_builder_def_const(bldr, "parse", opi_fn_new(parse_, 1));

  return 0;
}
