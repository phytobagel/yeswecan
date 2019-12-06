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
#include "MemoryStage.h"

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
   W * wreg = (W *) pregs[WREG];

   uint64_t E_icode = ereg->geticode()->getOutput(), 
            E_ifun = ereg->getifun()->getOutput(),
            e_Cnd = 0, 
            E_valA = ereg->getvalA()->getOutput(),
            E_valB = ereg->getvalB()->getOutput(),
            E_valC = ereg->getvalC()->getOutput(),
            E_dstE = ereg->getdstE()->getOutput(), 
            E_dstM = ereg->getdstM()->getOutput(), 
            E_stat = ereg->getstat()->getOutput();


   uint64_t ALU_A = aluA(E_icode, E_valA, E_valC);
   uint64_t ALU_B = aluB(E_icode, E_valB);
   uint64_t ALU_fun = alufun(E_icode, E_ifun);
   
   bool set_cc = ExecuteStage::set_cc(E_icode, stages, wreg);
   M_bubble = ExecuteStage::calculateControlSignals(stages, wreg);
 
   uint64_t ALUoutput = ALU(ALU_A, ALU_B, ALU_fun, set_cc);
   
   e_valE_var = ALUoutput;

   e_Cnd = cond(E_icode, E_ifun);

   e_dstE_var = e_dstE(E_icode, e_Cnd, E_dstE);

   setMInput(mreg, E_stat, E_icode, e_Cnd, ALUoutput, E_valA, e_dstE_var, E_dstM);
   return false;
} 

uint64_t ExecuteStage::cond(uint64_t E_icode, uint64_t E_ifun)
{
    ConditionCodes * cond = cond->getInstance();
    bool error = false;
    uint64_t sf = cond->getConditionCode(SF, error);
    uint64_t of = cond->getConditionCode(OF, error);
    uint64_t zf = cond->getConditionCode(ZF, error); 
    
    if ((E_icode == IJXX && E_ifun == UNCOND) || 
        (E_icode == IRRMOVQ && E_ifun == UNCOND)) return 1;

    if ((E_icode == IJXX && E_ifun == LESSEQ) || 
        (E_icode == IRRMOVQ && E_ifun == LESSEQ)) return ((sf != of) || zf);
   
    if ((E_icode == IJXX && E_ifun == LESS) || 
        (E_icode == IRRMOVQ && E_ifun == LESS)) return (sf != of);

    if ((E_icode == IJXX && E_ifun == EQUAL) || 
        (E_icode == IRRMOVQ && E_ifun == EQUAL)) return (zf);

    if ((E_icode == IJXX && E_ifun == NOTEQUAL) || 
        (E_icode == IRRMOVQ && E_ifun == NOTEQUAL)) return !(zf);

    if ((E_icode == IJXX && E_ifun == GREATER) || 
        (E_icode == IRRMOVQ && E_ifun == GREATER)) return (!(sf != of) && !zf);

    if ((E_icode == IJXX && E_ifun == GREATEREQ) || 
        (E_icode == IRRMOVQ && E_ifun == GREATEREQ)) return !(sf != of);
     
    return 0;
}
   
uint64_t ExecuteStage::gete_dstE()
{  
    return e_dstE_var;
}  
   
uint64_t ExecuteStage::gete_valE()
{
    return e_valE_var;
}

uint64_t ExecuteStage::ALU(uint64_t ALU_A, uint64_t ALU_B, uint64_t ALU_fun, bool set_cc)
{
    uint64_t result = 0;
    bool overflow = false;

    if (ALU_fun == ADDQ)
    {
        result = ALU_A + ALU_B;
        overflow = Tools::addOverflow(ALU_A, ALU_B);
    }
    else if (ALU_fun == SUBQ)
    {
        result = ALU_B - ALU_A;
        overflow = Tools::subOverflow(ALU_B, ALU_A);
    }
    else if (ALU_fun == ANDQ)
    {
        result = ALU_A & ALU_B;
        overflow = 0;
    }
    else if (ALU_fun == XORQ)
    {
        result = ALU_A ^ ALU_B;
        overflow = 0;
    }

    if(set_cc)
        ExecuteStage::CC(result, overflow);

    return result;
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

bool ExecuteStage::set_cc(uint64_t E_icode, Stage ** stages, W * wreg)
{
    uint64_t W_stat = wreg->getstat()->getOutput();
    MemoryStage * mStage =  (MemoryStage *) stages[MSTAGE];
    uint64_t m_stat = mStage->getm_stat();
    
    return (E_icode == IOPQ) &&
          !(m_stat == SADR ||
            m_stat == SINS ||
            m_stat == SHLT ) &&
          !(W_stat == SADR ||
            W_stat == SINS ||
            W_stat == SHLT);
}

bool ExecuteStage::calculateControlSignals(Stage ** stages, W * wreg)
{
    uint64_t W_stat = wreg->getstat()->getOutput();
    MemoryStage * mStage = (MemoryStage *) stages[MSTAGE];
    uint64_t m_stat = mStage->getm_stat();

    return (m_stat == SADR ||
            m_stat == SINS ||
            m_stat == SHLT) ||
           (W_stat == SADR ||
            W_stat == SINS ||
            W_stat == SHLT);
}

uint64_t ExecuteStage::e_dstE(uint64_t E_icode, bool e_Cnd, uint64_t E_dstE)
{
    if (E_icode == IRRMOVQ && !e_Cnd) return RNONE;
    return E_dstE;
}

void ExecuteStage::CC(uint64_t ALUoutput, bool overflow)
{

    ConditionCodes * cond = ConditionCodes::getInstance();
    bool error = false;

    cond->setConditionCode(overflow, OF, error); 

    cond->setConditionCode(Tools::sign(ALUoutput), SF, error);

    if(ALUoutput == 0)
        cond->setConditionCode(1, ZF, error);
    else
        cond->setConditionCode(0, ZF, error);

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

   if (!M_bubble)
   {
       mreg->getstat()->normal();
       mreg->geticode()->normal();
       mreg->getCnd()->normal();
       mreg->getvalE()->normal();
       mreg->getvalA()->normal();
       mreg->getdstE()->normal();
       mreg->getdstM()->normal();
   }
   else
   {
       mreg->getstat()->bubble(SAOK);
       mreg->geticode()->bubble(INOP);
       mreg->getCnd()->bubble();
       mreg->getvalE()->bubble();
       mreg->getvalA()->bubble();
       mreg->getdstE()->bubble(RNONE);
       mreg->getdstM()->bubble(RNONE);
   }
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
