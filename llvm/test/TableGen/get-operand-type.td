// RUN: llvm-tblgen -gen-instr-info -I %p/../../include %s | FileCheck %s

// Check that getOperandType has the expected info in it

include "llvm/Target/Target.td"

def archInstrInfo : InstrInfo { }

def arch : Target {
  let InstructionSet = archInstrInfo;
}

def Reg : Register<"reg">;
def RegClass : RegisterClass<"foo", [i32], 0, (add Reg)>;

def OpA : Operand<i32>;
def OpB : Operand<i32>;

def InstA : Instruction {
  let Size = 1;
  let OutOperandList = (outs OpA:$a);
  let InOperandList = (ins OpB:$b, i32imm:$c);
  field bits<8> Inst;
  field bits<8> SoftFail = 0;
  let Namespace = "MyNamespace";
}

def InstB : Instruction {
  let Size = 1;
  let OutOperandList = (outs i32imm:$d);
  let InOperandList = (ins unknown:$x);
  field bits<8> Inst;
  field bits<8> SoftFail = 0;
  let Namespace = "MyNamespace";
}

// CHECK: #ifdef GET_INSTRINFO_OPERAND_TYPE
// CHECK:        OpTypes::OpA, OpTypes::OpB, OpTypes::i32imm,
// CHECK-NEXT:   OpTypes::i32imm, -1,
// CHECK: #endif //GET_INSTRINFO_OPERAND_TYPE
