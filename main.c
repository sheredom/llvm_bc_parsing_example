// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>

#include <stdio.h>

#if defined(_MSC_VER)
#include <io.h>
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#endif

int main(const int argc, const char *const argv[]) {
  if (3 != argc) {
    fprintf(stderr, "Invalid command line!\n");
    return 1;
  }

  const char *const inputFilename = argv[1];
  const char *const outputFilename = argv[2];

  LLVMMemoryBufferRef memoryBuffer;

  // check if we are to read our input file from stdin
  if (('-' == inputFilename[0]) && ('\0' == inputFilename[1])) {
    char *message;
    if (0 != LLVMCreateMemoryBufferWithSTDIN(&memoryBuffer, &message)) {
      fprintf(stderr, "%s\n", message);
      free(message);
      return 1;
    }
  } else {
    char *message;
    if (0 != LLVMCreateMemoryBufferWithContentsOfFile(
                 inputFilename, &memoryBuffer, &message)) {
      fprintf(stderr, "%s\n", message);
      free(message);
      return 1;
    }
  }

  // now create our module using the memorybuffer
  LLVMModuleRef module;
  if (0 != LLVMParseBitcode2(memoryBuffer, &module)) {
    fprintf(stderr, "Invalid bitcode detected!\n");
    LLVMDisposeMemoryBuffer(memoryBuffer);
    return 1;
  }

  // done with the memory buffer now, so dispose of it
  LLVMDisposeMemoryBuffer(memoryBuffer);

  // loop through all the functions in the module
  LLVMValueRef function = LLVMGetFirstFunction(module);
  for (; function;
       function = LLVMGetNextFunction(function)) {
    // loop through all the basic blocks in the function
   LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function);
    for (;basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
      // we'll keep track of the last instruction that we seen (for reasons that
      // will become clearer later)
      LLVMValueRef lastInstruction = 0;

      // loop through all the instructions in the basic block
     LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock)
      for (;instruction;) {
        LLVMValueRef replacementValue = 0;

        // look for math instructions
        if (LLVMIsABinaryOperator(instruction)) {
          // we have a binary operator, which always has two operands
          LLVMValueRef x = LLVMGetOperand(instruction, 0);
          LLVMValueRef y = LLVMGetOperand(instruction, 1);

          // check if each argument is a constant
          const int allConstant = LLVMIsAConstant(x) && LLVMIsAConstant(y);

          if (allConstant) {
            switch (LLVMGetInstructionOpcode(instruction)) {
            default:
              break;
            case LLVMAdd:
              replacementValue = LLVMConstAdd(x, y);
              break;
            case LLVMFAdd:
              replacementValue = LLVMConstFAdd(x, y);
              break;
            case LLVMSub:
              replacementValue = LLVMConstSub(x, y);
              break;
            case LLVMFSub:
              replacementValue = LLVMConstFSub(x, y);
              break;
            case LLVMMul:
              replacementValue = LLVMConstMul(x, y);
              break;
            case LLVMFMul:
              replacementValue = LLVMConstFMul(x, y);
              break;
            case LLVMUDiv:
              replacementValue = LLVMConstUDiv(x, y);
              break;
            case LLVMSDiv:
              replacementValue = LLVMConstSDiv(x, y);
              break;
            case LLVMFDiv:
              replacementValue = LLVMConstFDiv(x, y);
              break;
            case LLVMURem:
              replacementValue = LLVMConstURem(x, y);
              break;
            case LLVMSRem:
              replacementValue = LLVMConstSRem(x, y);
              break;
            case LLVMFRem:
              replacementValue = LLVMConstFRem(x, y);
              break;
            case LLVMShl:
              replacementValue = LLVMConstShl(x, y);
              break;
            case LLVMLShr:
              replacementValue = LLVMConstLShr(x, y);
              break;
            case LLVMAShr:
              replacementValue = LLVMConstAShr(x, y);
              break;
            case LLVMAnd:
              replacementValue = LLVMConstAnd(x, y);
              break;
            case LLVMOr:
              replacementValue = LLVMConstOr(x, y);
              break;
            case LLVMXor:
              replacementValue = LLVMConstXor(x, y);
              break;
            }
          }
        }

        // if we managed to find a more optimal replacement
        if (replacementValue) {
          // replace all uses of the old instruction with the new one
          LLVMReplaceAllUsesWith(instruction, replacementValue);

          // erase the instruction that we've replaced
          LLVMInstructionEraseFromParent(instruction);

          // if we don't have a previous instruction, get the first one from the
          // basic block again
          if (!lastInstruction) {
            instruction = LLVMGetFirstInstruction(basicBlock);
          } else {
            instruction = LLVMGetNextInstruction(lastInstruction);
          }
        } else {
          lastInstruction = instruction;
          instruction = LLVMGetNextInstruction(instruction);
        }
      }
    }
  }

  // check if we are to write our output file to stdout
  if (('-' == outputFilename[0]) && ('\0' == outputFilename[1])) {
    if (0 != LLVMWriteBitcodeToFD(module, STDOUT_FILENO, 0, 0)) {
      fprintf(stderr, "Failed to write bitcode to stdout!\n");
      LLVMDisposeModule(module);
      return 1;
    }
  } else {
    if (0 != LLVMWriteBitcodeToFile(module, outputFilename)) {
      fprintf(stderr, "Failed to write bitcode to file\n");
      LLVMDisposeModule(module);
      return 1;
    }
  }

  LLVMDisposeModule(module);

  return 0;
}
