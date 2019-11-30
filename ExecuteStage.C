#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ConditionCodes.h"
#include "Tools.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];
   uint64_t E_icode = ereg->geticode()->getOutput(), 
            E_ifun = ereg->getifun()->getOutput(),
            e_Cnd = 0, 
            E_valA = ereg->getvalA()->getOutput(),
            E_valB = ereg->getvalB()->getOutput(),
            E_valC = ereg->getvalC()->getOutput(),
            E_dstE = ereg->getdstE()->getOutput(), 
            E_dstM = ereg->getdstM()->getOutput(), 
            E_stat = SAOK;


   uint64_t ALU_A = aluA(E_icode, E_valA, E_valC);
   uint64_t ALU_B = aluB(E_icode, E_valB);
   uint64_t ALU_fun = E_ifun;
   
   uint64_t ALUoutput = ALU(ALU_A, ALU_B, ALU_fun);
   uint64_t e_dstE = E_dstE;

   setMInput(mreg, E_stat, E_icode, e_Cnd, ALUoutput, E_valA, e_dstE, E_dstM);
   return false;
}

uint64_t ExecuteStage::ALU(uint64_t ALU_A, uint64_t ALU_B, uint64_t ALU_fun)
{
    if (ALU_fun == 0)
        return ALU_A + ALU_B;
    else if (ALU_fun == 1)
        return ALU_B - ALU_A;
    else if (ALU_fun == 2)
        return ALU_A & ALU_B;
    else if (ALU_fun == 3)
        return ALU_A ^ ALU_B;

    return -1;
}

uint64_t ExecuteStage::aluA(uint64_t E_icode, uint64_t E_valA, uint64_t E_valC)
{
    if (E_icode == IRRMOVQ ||
        E_icode == IOPQ)
    {
        return E_valA;
    }
    else if (E_icode == IIRMOVQ ||
             E_icode == IRMMOVQ ||
             E_icode == IMRMOVQ)
    {
        return E_valC;
    }
    else if (E_icode == ICALL ||
             E_icode == IPUSHQ)
    {
        return -8;
    }
    else if (E_icode == IRET ||
             E_icode == IPOPQ)
    {
        return 8;
    }

    return 0;
}

uint64_t ExecuteStage::aluB(uint64_t E_icode, uint64_t E_valB)
{
    if (E_icode == IRMMOVQ ||
        E_icode == IMRMOVQ ||
        E_icode == IOPQ    ||
        E_icode == ICALL   ||
        E_icode == IPUSHQ  ||
        E_icode == IRET    ||
        E_icode == IPOPQ)
    {
        return E_valB;
    }
    else if (E_icode == IRRMOVQ ||
             E_icode == IIRMOVQ)
    {
        return 0;
    }

    return 0;
}

uint64_t ExecuteStage::alufun(uint64_t E_icode, uint64_t E_ifun)
{
    if (E_icode == IOPQ) return E_ifun;
    return ADDQ;
}

bool ExecuteStage::set_cc(uint64_t E_icode)
{
    if (E_icode == IOPQ) return true;
    return false;
}

uint64_t ExecuteStage::e_dstE(uint64_t E_icode, bool e_Cnd, uint64_t E_dstE)
{
    if (E_icode == IRRMOVQ && !e_Cnd) return RNONE;
    return E_dstE;
}

uint64_t ExecuteStage::CC(bool set_cc, uint64_t ALUoutput, uint64_t ALU_A, uint64_t ALU_B)
{
    if (!set_cc) return false;

    ConditionCodes * cond = ConditionCodes::getInstance();
    bool error = false;

    if ((Tools::sign(ALUoutput) == 0 || ALUoutput == 0) &&
        Tools::sign(ALU_A) == 1 && Tools::sign(ALU_B) == 1)
        cond->setConditionCode(1, OF, error);
    
    if ((Tools::sign(ALUoutput) == 1 || ALUoutput == 0) &&
        Tools::sign(ALU_A) == 0 && Tools::sign(ALU_B) == 0)
        cond->setConditionCode(1, OF, error);

    if (Tools::sign(ALUoutput) == 1) cond->setConditionCode(1, SF, error);
    if (ALUoutput == 0) cond->setConditionCode(1, ZF, error);

    return true;
}


/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
   M * mreg = (M *) pregs[MREG];

   mreg->getstat()->normal();
   mreg->geticode()->normal();
   mreg->getCnd()->normal();
   mreg->getvalE()->normal();
   mreg->getvalA()->normal();
   mreg->getdstE()->normal();
   mreg->getdstM()->normal();
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode,
                             uint64_t cnd, uint64_t valE, uint64_t valA,
                             uint64_t dstE, uint64_t dstM)
{
   mreg->getstat()->setInput(stat);
   mreg->geticode()->setInput(icode);
   mreg->getCnd()->setInput(cnd);
   mreg->getvalE()->setInput(valE);
   mreg->getvalA()->setInput(valA);
   mreg->getdstE()->setInput(dstE);
   mreg->getdstM()->setInput(dstM);
}
