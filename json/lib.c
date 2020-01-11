#include "opium/opium.h"
#include "cjson/cJSON.h"

static opi_t
json_to_opi(const cJSON *json)
{
  if (cJSON_IsFalse(json)) {
    return opi_false;
  } else if (cJSON_IsTrue(json)) {
    return opi_true;
  } else if (cJSON_IsNull(json)) {
    return opi_nil;
  } else if (cJSON_IsNumber(json)) {
    return opi_num_new(json->valuedouble);
  } else if (cJSON_IsString(json)) {
    return opi_str_new(json->valuestring);
  } else if (cJSON_IsArray(json)) {
    int len = cJSON_GetArraySize(json);
    opi_t *data = malloc(sizeof(opi_t) * len);
    const cJSON *elt = NULL;
    int i = 0;
    cJSON_ArrayForEach(elt, json) {
      opi_inc_rc(data[i++] = json_to_opi(elt));
    }
    return opi_array_drain(data, len, len);
  } else if (cJSON_IsObject(json)) {
    opi_t l = opi_nil;
    const cJSON *elt = NULL;
    cJSON_ArrayForEach(elt, json) {
      opi_t key = opi_str_new(elt->string);
      opi_t val = json_to_opi(elt);
      l = opi_cons(opi_cons(key, val), l);
    }
    return opi_table(l, FALSE);
  } else {
    opi_error("unexpected JSON object type\n");
    abort();
  }
}

static
OPI_DEF(parse,
  opi_arg(str, opi_str_type)
  cJSON *json = cJSON_Parse(OPI_STR(str)->str);
  if (!json)
    opi_throw("json-error");
  opi_return(json_to_opi(json));
)

int
opium_library(OpiBuilder * bldr)
{
  opi_builder_def_const(bldr, "parse", opi_fn_new(parse, 1));
  return 0;
}
