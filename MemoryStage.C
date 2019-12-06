#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   W * wreg = (W *) pregs[WREG];
   M * mreg = (M *) pregs[MREG];
   Memory * mem = mem->getInstance();
   bool error = false;
   
   uint64_t M_icode = mreg->geticode()->getOutput(),
            M_valE = mreg->getvalE()->getOutput() ,
            M_valA = mreg->getvalA()->getOutput();
   uint64_t M_dstE = mreg->getdstE()->getOutput(),
            M_dstM = mreg->getdstM()->getOutput(),
            M_stat = SAOK;

   uint64_t m_stat = M_stat;
   m_valM_var = 0;   
   uint64_t address = mem_addr(M_icode, M_valE, M_valA);  

   if (mem_read(M_icode))
       m_valM_var = mem->getLong(address, error); 

   if (mem_write(M_icode))
       mem->putLong(M_valA, address, error);

   setWInput(wreg, m_stat, M_icode, M_valE, m_valM_var, M_dstE, M_dstM);
   return false;
}

uint64_t MemoryStage::getm_valM()
{
    return m_valM_var;
}

bool MemoryStage::mem_read(uint64_t M_icode)
{
    return (M_icode == IMRMOVQ ||
            M_icode == IPOPQ ||
            M_icode == IRET);
}

bool MemoryStage::mem_write(uint64_t M_icode)
{
    return (M_icode == IRMMOVQ ||
            M_icode == IPUSHQ ||
            M_icode == ICALL);
}

uint64_t MemoryStage::mem_addr(uint64_t M_icode, uint64_t M_valE, uint64_t M_valA)
{
    if (M_icode == IRMMOVQ ||
        M_icode == IPUSHQ || 
        M_icode == ICALL  ||
        M_icode == IMRMOVQ) return M_valE;

    if (M_icode == IPOPQ ||
        M_icode == IRET) return M_valA;

    return 0;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
   W * wreg = (W *) pregs[WREG];

   wreg->getstat()->normal();
   wreg->geticode()->normal();
   wreg->getvalE()->normal();
   wreg->getvalM()->normal();
   wreg->getdstE()->normal();
   wreg->getdstM()->normal();
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
void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode,
                                      uint64_t valE, uint64_t valM,
                                      uint64_t dstE, uint64_t dstM)
{
   wreg->getstat()->setInput(stat);
   wreg->geticode()->setInput(icode);
   wreg->getvalE()->setInput(valE);
   wreg->getvalM()->setInput(valM);
   wreg->getdstE()->setInput(dstE);
   wreg->getdstM()->setInput(dstM);
}
