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
    return opi_string_new(json->valuestring);
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
      opi_t key = opi_string_new(elt->string);
      opi_t val = json_to_opi(elt);
      l = opi_cons(opi_cons(key, val), l);
    }
    return opi_table(l, FALSE);
  } else {
    opi_error("unexpected JSON object type\n");
    abort();
  }
}

static opi_t
json_parse(void)
{
  OPI_FN()
  OPI_ARG(str, opi_string_type)
  cJSON *json = cJSON_Parse(OPI_STR(str)->str);
  if (!json)
    OPI_THROW("json-error");
  OPI_RETURN(json_to_opi(json));
}

int
opium_library(OpiBuilder * bldr)
{
  opi_builder_def_const(bldr, "__JSON_parse", opi_fn(NULL, json_parse, 1));
  return 0;
}
