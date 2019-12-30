#include "opium/opium.h"
#include <llvm-c/Core.h>

static opi_type_t module_type;
typedef struct Module_s {
  OpiHeader header;
  LLVMModuleRef mod;
  int is_owner;
} Module;
#define MODULE(x) ((Module*)(x))

static void
module_delete(opi_type_t type, opi_t x)
{
  if (MODULE(x)->is_owner)
    LLVMDisposeModule(MODULE(x)->mod);
  free(x);
}

static opi_t
module_new(LLVMModuleRef mod, int is_owner)
{
  Module *m = malloc(sizeof(Module));
  m->mod = mod;
  m->is_owner = is_owner;
  opi_init_cell(m, module_type);
  return OPI(m);
}

static
OPI_DEF(Module_createWithName,
  opi_arg(name, opi_string_type)
  LLVMModuleRef mod = LLVMModuleCreateWithName(OPI_STR(name)->str);
  opi_return(module_new(mod, TRUE));
)

static
OPI_DEF(Module_dump,
  opi_arg(m, module_type)
  LLVMDumpModule(MODULE(m)->mod);
)

static
OPI_DEF(Module_printToString,
  opi_arg(m, module_type)
  char *s = LLVMPrintModuleToString(MODULE(m)->mod);
  opi_t ret = opi_string_new(s);
  LLVMDisposeMessage(s);
  opi_return(ret);
)

int
opium_library(OpiBuilder *bldr)
{
  module_type = opi_type_new("Module");
  opi_type_set_delete_cell(module_type, module_delete);

  opi_builder_def_const(bldr, "Module.createWithName", opi_fn(0, Module_createWithName, 1));
  opi_builder_def_const(bldr, "Module.dump", opi_fn(0, Module_dump, 1));
  opi_builder_def_const(bldr, "Module.printToString", opi_fn(0, Module_printToString, 1));

  return 0;
}
