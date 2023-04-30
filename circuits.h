//****************************************************************
//Contains All Circuits used in the ALU CONTROL and CONTROL UNITS*
//****************************************************************
typedef char BIT;
#define TRUE 1
#define FALSE 0
#define UNDEF -1

BIT not_gate(BIT A);
BIT or_gate(BIT A, BIT B);
BIT or_gate3(BIT A, BIT B, BIT C);
BIT and_gate(BIT A, BIT B);
BIT and_gate3(BIT A, BIT B, BIT C);
BIT xor_gate(BIT A, BIT B);
BIT nor_gate(BIT A, BIT B);
BIT nand_gate(BIT A, BIT B);

// CONTROL CIRCUITS
BIT MemToRegCircuit(BIT* OpCode){
    BIT return_bit = and_gate3(OpCode[0], OpCode[1], \
                and_gate3(not_gate(OpCode[2]), not_gate(OpCode[3]), \
                and_gate(not_gate(OpCode[4]), OpCode[5])));
    return return_bit;
}
BIT MemWriteCircuit(BIT* OpCode){
    BIT return_bit = and_gate3(OpCode[0], OpCode[1], \
                and_gate3(not_gate(OpCode[2]), OpCode[3], \
                and_gate(not_gate(OpCode[4]), OpCode[5])));
    return return_bit;
}

BIT BranchCircuit(BIT* OpCode){
    BIT return_bit =  and_gate3(not_gate(OpCode[0]), not_gate(OpCode[1]), \
                and_gate3(OpCode[2], not_gate(OpCode[3]), \
                and_gate(not_gate(OpCode[4]), not_gate(OpCode[5]) )));
    return return_bit;
}

BIT RegDstCircuit(BIT* OpCode){
    BIT return_bit =  and_gate3(not_gate(OpCode[0]), not_gate(OpCode[1]), \
                and_gate3(not_gate(OpCode[2]), not_gate(OpCode[3]), \
                and_gate(not_gate(OpCode[4]), not_gate(OpCode[5]) )));
    return return_bit;
}

BIT LinkCircuit(BIT* OpCode){
    BIT return_bit =  and_gate3(OpCode[0], OpCode[1], \
                and_gate3(not_gate(OpCode[2]), not_gate(OpCode[3]), \
                and_gate(not_gate(OpCode[4]), not_gate(OpCode[5]) )));
    return return_bit;
}

BIT JumpCircuit(BIT* OpCode){
    BIT return_bit =  and_gate3(not_gate(OpCode[0]), OpCode[1], \
                and_gate3(not_gate(OpCode[2]), not_gate(OpCode[3]), \
                and_gate(not_gate(OpCode[4]), not_gate(OpCode[5]) )));
    return return_bit;

}

BIT AddiCircuit(BIT * OpCode){
    BIT return_bit =  and_gate3(not_gate(OpCode[0]), not_gate(OpCode[1]), \
                and_gate3(not_gate(OpCode[2]), OpCode[3], \
                and_gate(not_gate(OpCode[4]), not_gate(OpCode[5]) )));
    return return_bit;
}

// ALU CONTROL CIRCUITS

BIT JumpRegCircuit(BIT * funct){
    BIT return_bit = and_gate3(not_gate(funct[0]), not_gate(funct[1]), \
                and_gate3(not_gate(funct[2]), funct[3], \
                and_gate(not_gate(funct[4]), not_gate(funct[5]))));
    return return_bit;
}

BIT AluControl_Circuit0(BIT * funct){
    BIT return_bit = and_gate3(funct[0], not_gate(funct[1]), \
                and_gate3(funct[2], not_gate(funct[3]), \
                and_gate(not_gate(funct[4]), funct[5])));
    return return_bit;

}

BIT AluControl_Circuit1(BIT * funct){
    BIT return_bit = and_gate3(not_gate(funct[0]), not_gate(funct[1]), \
                and_gate3(funct[2], not_gate(funct[3]), \
                and_gate(not_gate(funct[4]), funct[5])));
    return return_bit;
}
BIT BinvertCircuit(BIT * funct){
    BIT return_bit = and_gate3(not_gate(funct[0]), not_gate(funct[1]), \
                and_gate3(not_gate(funct[2]), not_gate(funct[3]), \
                and_gate(not_gate(funct[4]), funct[5])));
    return return_bit;

}
BIT LessCircuit(BIT * funct){
    BIT return_bit = and_gate3(not_gate(funct[0]), funct[1], \
                and_gate3(not_gate(funct[2]), funct[3], \
                and_gate(not_gate(funct[4]), funct[5])));
    return return_bit;
}
