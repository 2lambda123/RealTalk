
#include <cassert>
#include <string>
#include <vector>
#include "real_talk/code/cmd.h"
#include "real_talk/code/module.h"
#include "real_talk/code/module_reader.h"
#include "real_talk/code/code.h"

using std::string;
using std::vector;
using std::ios;
using std::istream;
using std::unique_ptr;

namespace real_talk {
namespace code {

Module ModuleReader::ReadFromCode(Code *module_code) {
  assert(module_code);
  const uint32_t version = module_code->ReadUint32();
  const uint32_t cmds_address = module_code->ReadUint32();
  const uint32_t main_cmds_size = module_code->ReadUint32();
  const uint32_t func_cmds_size = module_code->ReadUint32();
  const uint32_t global_var_defs_metadata_address = module_code->ReadUint32();
  const uint32_t global_var_defs_metadata_size = module_code->ReadUint32();
  const uint32_t func_defs_metadata_address = module_code->ReadUint32();
  const uint32_t func_defs_metadata_size = module_code->ReadUint32();
  const uint32_t native_func_defs_metadata_address = module_code->ReadUint32();
  const uint32_t native_func_defs_metadata_size = module_code->ReadUint32();
  const uint32_t global_var_refs_metadata_address = module_code->ReadUint32();
  const uint32_t global_var_refs_metadata_size = module_code->ReadUint32();
  const uint32_t func_refs_metadata_address = module_code->ReadUint32();
  const uint32_t func_refs_metadata_size = module_code->ReadUint32();
  const uint32_t native_func_refs_metadata_address = module_code->ReadUint32();
  const uint32_t native_func_refs_metadata_size = module_code->ReadUint32();

  module_code->SetPosition(cmds_address);
  unique_ptr<Code> cmds_code(new Code(
      module_code->GetDataAtPosition(), main_cmds_size + func_cmds_size));

  module_code->SetPosition(global_var_defs_metadata_address);
  vector<string> ids_of_global_var_defs;
  const unsigned char * const global_var_defs_metadata_end =
      module_code->GetDataAtPosition() + global_var_defs_metadata_size;

  while (module_code->GetDataAtPosition() != global_var_defs_metadata_end) {
    ids_of_global_var_defs.push_back(module_code->ReadString());
  }

  module_code->SetPosition(func_defs_metadata_address);
  vector<IdAddress> id_addresses_of_func_defs;
  const unsigned char * const func_defs_metadata_end =
      module_code->GetDataAtPosition() + func_defs_metadata_size;

  while (module_code->GetDataAtPosition() != func_defs_metadata_end) {
    id_addresses_of_func_defs.push_back(module_code->ReadIdAddress());
  }

  module_code->SetPosition(native_func_defs_metadata_address);
  vector<string> ids_of_native_func_defs;
  const unsigned char * const native_func_defs_metadata_end =
      module_code->GetDataAtPosition() + native_func_defs_metadata_size;

  while (module_code->GetDataAtPosition() != native_func_defs_metadata_end) {
    ids_of_native_func_defs.push_back(module_code->ReadString());
  }

  module_code->SetPosition(global_var_refs_metadata_address);
  vector<IdAddresses> id_addresses_of_global_var_refs;
  const unsigned char * const global_var_refs_metadata_end =
      module_code->GetDataAtPosition() + global_var_refs_metadata_size;

  while (module_code->GetDataAtPosition() != global_var_refs_metadata_end) {
    id_addresses_of_global_var_refs.push_back(module_code->ReadIdAddresses());
  }

  module_code->SetPosition(func_refs_metadata_address);
  vector<IdAddresses> id_addresses_of_func_refs;
  const unsigned char * const func_refs_metadata_end =
      module_code->GetDataAtPosition() + func_refs_metadata_size;

  while (module_code->GetDataAtPosition() != func_refs_metadata_end) {
    id_addresses_of_func_refs.push_back(module_code->ReadIdAddresses());
  }

  module_code->SetPosition(native_func_refs_metadata_address);
  vector<IdAddresses> id_addresses_of_native_func_refs;
  const unsigned char * const native_func_refs_metadata_end =
      module_code->GetDataAtPosition() + native_func_refs_metadata_size;

  while (module_code->GetDataAtPosition() != native_func_refs_metadata_end) {
    id_addresses_of_native_func_refs.push_back(module_code->ReadIdAddresses());
  }

  return Module(version,
                move(cmds_code),
                main_cmds_size,
                id_addresses_of_func_defs,
                ids_of_global_var_defs,
                ids_of_native_func_defs,
                id_addresses_of_func_refs,
                id_addresses_of_native_func_refs,
                id_addresses_of_global_var_refs);
}

Module ModuleReader::ReadFromStream(istream *code_stream) {
  assert(code_stream);
  Code module_code(*code_stream);
  return ReadFromCode(&module_code);
}
}
}
