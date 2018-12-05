
#include "vm_core_extra/custom-code.h"
#include "vm_core_extra/code-page-chunk.h"
#include "vm_core/modules/assembler/assembler.h"

using namespace zz;

#if V8_TARGET_ARCH_ARM
using namespace zz::arm;
#elif V8_TARGET_ARCH_ARM64
using namespace zz::arm64;
#endif

AssemblerCode *AssemblerCode::FinalizeTurboAssembler(AssemblerBase *assembler) {
  TurboAssembler *turbo_assembler = reinterpret_cast<TurboAssembler *>(assembler);
  int code_size                   = turbo_assembler->CodeSize();

// Allocate the executable memory
#if V8_TARGET_ARCH_ARM64 || V8_TARGET_ARCH_ARM
  // extra bytes for align needed
  MemoryRegion *code_region = CodeChunk::AllocateCode(code_size + 4);
#else
  MemoryRegion *code_region = CodeChunk::AllocateCode(code_size);
#endif

  void *code_address = code_region->pointer();
  // Realize(Relocate) the buffer_code to the executable_memory_address, remove the ExternalLabels, etc, the pc-relative instructions
  turbo_assembler->CommitRealize(code_address);
  CodeChunk::PatchCodeBuffer(turbo_assembler->ReleaseAddress(), turbo_assembler->GetCodeBuffer());
  Code *code = turbo_assembler->GetCode();
  DLOG("[*] AssemblerCode finalize assembler at %p\n", code->raw_instruction_start());
  return reinterpret_cast<AssemblerCode *>(code);
}
