#include <assert.h>
#include <stdio.h> 
#include "circuits.h"

typedef char BIT;
#define TRUE 1
#define FALSE 0
#define UNDEF -1

BIT RegDst    = FALSE;
BIT Jump      = FALSE; BIT Branch    = FALSE;
BIT MemToReg  = FALSE;
BIT ALUOp[2]  = {FALSE};
BIT MemWrite  = FALSE;
BIT ALUImm    = FALSE;
BIT RegWrite  = FALSE;
BIT Zero      = FALSE;
BIT Link      = FALSE;
BIT JumpReg   = FALSE;
BIT ALUControl[4] = {FALSE};

BIT not_gate(BIT A)
{
  return (!A);
}

BIT or_gate(BIT A, BIT B)
{
  return (A || B);
}

BIT or_gate3(BIT A, BIT B, BIT C)
{
  return or_gate(A, or_gate(B, C));
}

BIT and_gate(BIT A, BIT B)
{
  return (A && B);
}

BIT and_gate3(BIT A, BIT B, BIT C)
{
  return and_gate(A, and_gate(B, C));
}

BIT xor_gate(BIT A, BIT B)
{
  BIT nA = not_gate(A);
  BIT nB = not_gate(B);
  BIT x0 = and_gate(A, nB);
  BIT x1 = and_gate(nA, B);
  return or_gate(x0, x1);
}

BIT nor_gate(BIT A, BIT B)
{
  return not_gate(or_gate(A, B));
}

BIT nand_gate(BIT A, BIT B)
{
  return not_gate(and_gate(A, B));
}

void decoder2(BIT* I, BIT EN, BIT* O)
{
  BIT nI1 = not_gate(I[1]);
  BIT nI0 = not_gate(I[0]);
  O[0] = and_gate(nI1, nI0);
  O[1] = and_gate(nI1, I[0]);
  O[2] = and_gate(I[1], nI0);
  O[3] = and_gate(I[1],  I[0]);
  
  // Note use of EN (enable) line below
  for (int i = 0; i < 4; ++i)
    O[i] = and_gate(EN, O[i]);
  
  return;
}

void decoder3(BIT* I, BIT EN, BIT* O)
{
  O[0] = and_gate3(not_gate(I[2]), not_gate(I[1]), not_gate(I[0]));
  O[1] = and_gate3(not_gate(I[2]), not_gate(I[1]), I[0]);
  O[2] = and_gate3(not_gate(I[2]), I[1], not_gate(I[0]));
  O[3] = and_gate3(not_gate(I[2]), I[1], I[0]);
  O[4] = and_gate3(I[2], not_gate(I[1]), not_gate(I[0]));
  O[5] = and_gate3(I[2], not_gate(I[1]), I[0]);
  O[6] = and_gate3(I[2], I[1], not_gate(I[0]));
  O[7] = and_gate3(I[2], I[1], I[0]);
  
  for (int i = 0; i < 8; ++i)
    O[i] = and_gate(EN, O[i]);
  
  return;
}

void decoder5(BIT* I, BIT EN, BIT* O)
{
   BIT EN_O[4] = {FALSE};
   decoder2(&I[3], EN, EN_O);
   decoder3(I, EN_O[3], &O[24]);
   decoder3(I, EN_O[2], &O[16]);
   decoder3(I, EN_O[1], &O[8]);
   decoder3(I, EN_O[0], &O[0]);
   
  for (int i = 0; i < 32; ++i)
    O[i] = and_gate(EN, O[i]);
  
}

BIT multiplexor2(BIT S, BIT I0, BIT I1)
{
  BIT nS = not_gate(S);
  BIT x0 = and_gate(nS, I0);
  BIT x1 = and_gate(S,  I1);
  return or_gate(x0, x1);
}

void multiplexor2_32(BIT S, BIT* I0, BIT* I1, BIT* Output)
{
  for (int i = 0; i < 32; ++i) {
    BIT nS = not_gate(S);
    BIT x0 = and_gate(nS, I0[i]);
    BIT x1 = and_gate(S, I1[i]);
    Output[i] = or_gate(x0, x1);
  }
}

BIT multiplexor4(BIT S0, BIT S1, BIT I0, BIT I1, BIT I2, BIT I3)
{
  BIT x0 , x1, x2, x3 = FALSE;
  BIT S[] = {S0, S1};
  BIT O[] = {I0,I1,I2,I3};
  decoder2(S,TRUE, O);
  
  BIT y0 = and_gate(x0, I0);
  BIT y1 = and_gate(x1, I1);
  BIT y2 = and_gate(x2, I2);
  BIT y3 = and_gate(x3, I3);
  
  BIT z0 = or_gate(y0, y1);
  BIT z1 = or_gate(y2, y3);
  
  return or_gate(z0, z1);
}

void Control(BIT* OpCode) {
  // TODO: Set control bits for everything
  // Input: opcode field from the instruction
  // OUtput: all control lines get set 
  // Note: Can use SOP or similar approaches to determine bits
  MemToReg = MemToRegCircuit(OpCode);
  MemWrite = MemWriteCircuit(OpCode);
  Branch   = BranchCircuit(OpCode);
  RegDst   = RegDstCircuit(OpCode);
  Link     = LinkCircuit(OpCode);
  RegWrite = or_gate3(RegDst, Link, or_gate(MemToReg, AddiCircuit(OpCode)));
  Jump     = or_gate(JumpCircuit(OpCode), Link);
  ALUOp[0] = or_gate3(AddiCircuit(OpCode), MemToReg, MemWrite);
  ALUOp[1] = Branch;
  ALUImm   = ALUOp[0];
}

void updateAluControl(){
    //ALUOp != {0,0} we have to update the alu contorl bits
    BIT ADD[] = {0, 0, 1, 0};
    BIT SUB[] = {0, 1, 1, 0};
    for(int i=0; i<4; i++)
        ALUControl[i] = multiplexor2(ALUOp[0], ALUControl[i], ADD[i]);
    for(int i=0; i<4; i++)
        ALUControl[i] = multiplexor2(ALUOp[1], ALUControl[i], SUB[i]);
}

void ALU_Control(BIT* funct)
{

  JumpReg  = and_gate3(JumpRegCircuit(funct),   \
          not_gate(or_gate(ALUOp[0], ALUOp[1])), RegDst);
  //Set RegWrite to 0 if we are preforming a jr instruction
  RegWrite = multiplexor2(JumpReg, RegWrite, 0);
  ALUControl[3] = AluControl_Circuit0(funct);
  ALUControl[2] = not_gate(or_gate(AluControl_Circuit1(funct), ALUControl[3]));
  ALUControl[1] = not_gate(or_gate(not_gate(ALUControl[2]), BinvertCircuit(funct)));
  ALUControl[0] = LessCircuit(funct);
  updateAluControl();
}

int main(){
    BIT lw  [] = {1,1,0,0,0,1};
    BIT sw  [] = {1,1,0,1,0,1};
    BIT beq [] = {0,0,1,0,0,0};
    BIT addi[] = {0,0,0,1,0,0};
    BIT R   [] = {0,0,0,0,0,0};
    BIT J   [] = {0,1,0,0,0,0};
    BIT JAL [] = {1,1,0,0,0,0};

    BIT T_funct [] = {0,0,0,1,0,0};

    /****************************************************************************************************************************/
    /* TEST LOAD WORD*/
    /****************************************************************************************************************************/
    Control(lw);
    ALU_Control(T_funct);

    assert(!ALUOp[1] && ALUOp[0] && MemToReg && !RegDst && ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(!ALUControl[0] && !ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/


    /****************************************************************************************************************************/
    /* TEST STORE WORD*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    Control(sw);
    ALU_Control(T_funct);

    assert(!ALUOp[1] && ALUOp[0] && !MemToReg && !RegDst && ALUImm \
            && !Branch && !Jump && !Link && !RegWrite && MemWrite);
    assert(!ALUControl[0] && !ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    
    /****************************************************************************************************************************/
    /* TEST BEQ*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    Control(beq);
    ALU_Control(T_funct);

    assert(ALUOp[1] && !ALUOp[0] && !MemToReg && !RegDst && !ALUImm \
            && Branch && !Jump && !Link && !RegWrite && !MemWrite);
    assert(!ALUControl[0] && ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/


    /****************************************************************************************************************************/
    /* TEST addi*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    Control(addi);
    ALU_Control(T_funct);

    assert(!ALUOp[1] && ALUOp[0] && !MemToReg && !RegDst && ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(!ALUControl[0] && !ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    /****************************************************************************************************************************/
    /* TEST and*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    BIT and [] = {0,0,1,0,0,1};
    Control(R);
    ALU_Control(and);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && RegDst && !ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(!ALUControl[0] && !ALUControl[1] && \
            !ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/


    /****************************************************************************************************************************/
    /* Test or*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    BIT or [] = {1,0,1,0,0,1};
    Control(R);
    ALU_Control(or);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && RegDst && !ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(!ALUControl[0] && !ALUControl[1] && \
            !ALUControl[2] && ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    /****************************************************************************************************************************/
    /* Test add*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    BIT add [] = {0,0,0,0,0,1};
    Control(R);
    ALU_Control(add);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && RegDst && !ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(!ALUControl[0] && !ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    /****************************************************************************************************************************/
    /* Test sub*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    BIT sub [] = {0,1,0,0,0,1};
    Control(R);
    ALU_Control(sub);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && RegDst && !ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(!ALUControl[0] && ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    /****************************************************************************************************************************/
    /* Test slt*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    BIT slt [] = {0,1,0,1,0,1};
    Control(R);
    ALU_Control(slt);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && RegDst && !ALUImm \
            && !Branch && !Jump && !Link && RegWrite && !MemWrite);
    assert(ALUControl[0] && ALUControl[1] && \
            ALUControl[2] && !ALUControl[3]);
    assert(!JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    /****************************************************************************************************************************/
    /* Test jr*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    BIT jr [] = {0,0,0,1,0,0};
    Control(R);
    ALU_Control(jr);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && RegDst && !ALUImm \
            && !Branch && !Jump && !Link && !RegWrite && !MemWrite);
    assert(JumpReg);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/


    /****************************************************************************************************************************/
    /* TEST jump*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    Control(J);

    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && !RegDst && !ALUImm \
            && !Branch && Jump && !Link && !RegWrite && !MemWrite);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    /****************************************************************************************************************************/
    /* TEST jal*/
    /****************************************************************************************************************************/
    RegDst = Jump = Branch = MemToReg = ALUOp[0] = ALUOp[1] = MemWrite = ALUImm = RegWrite = Link = FALSE;

    Control(JAL);


    assert(!ALUOp[1] && !ALUOp[0] && !MemToReg && !RegDst && !ALUImm \
            && !Branch && Jump && Link && RegWrite && !MemWrite);

    printf("PASSED\n");
    /****************************************************************************************************************************/
    /****************************************************************************************************************************/

    return 0;
}
