#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "Instructions.h"
#include "E.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];
   RegisterFile * reg = RegisterFile::getInstance();
   bool error = false;

   uint64_t icode = dreg->geticode()->getOutput(), 
            ifun = dreg->getifun()->getOutput(), 
            valC = dreg->getvalC()->getOutput(), 
            valA = 0, 
            valB = 0;
   uint64_t dstE = RNONE, dstM = RNONE, stat = SAOK, srcA = RNONE, srcB = RNONE;

   srcA = getSrcA(icode,dreg->getrA()->getOutput());
   //srcB = getSrcB();
   //dstE = getDstE();
   //dstM = getDstM();
   valA = reg->readRegister(srcA,error);
   valB = reg->readRegister(srcB,error);

   //get d_srcA 2 calls to src Reg and fwd's

   

   //provide the input values for the D register
   setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
   E * ereg = (E *) pregs[EREG];

   ereg->getstat()->normal();
   ereg->geticode()->normal();
   ereg->getifun()->normal();
   ereg->getvalC()->normal();
   ereg->getvalA()->normal();
   ereg->getvalB()->normal();
   ereg->getdstE()->normal();
   ereg->getdstM()->normal();
   ereg->getsrcA()->normal();
   ereg->getsrcB()->normal();
}

/* setEInput
 * provides the input to potentially be stored in the E register
 * during doClockHigh
 *
 * @param: ereg - pointer to the E register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM,
                           uint64_t srcA, uint64_t srcB)
{
   ereg->getstat()->setInput(stat);
   ereg->geticode()->setInput(icode);
   ereg->getifun()->setInput(ifun);
   ereg->getvalC()->setInput(valC);
   ereg->getvalA()->setInput(valA);
   ereg->getvalB()->setInput(valB);
   ereg->getdstE()->setInput(dstE);
   ereg->getdstM()->setInput(dstM);
   ereg->getsrcA()->setInput(srcA);
   ereg->getsrcB()->setInput(srcB);
}
 
/* setSrcA
 * sets the d_srcA component based on the icode
 * and sets dreg to RNONE
 *
 * @param D_icode
 * @param 
 */
 uint64_t DecodeStage::getSrcA(uint64_t D_icode, uint64_t D_rA)
 {
     if(D_icode == IRRMOVQ ||
        D_icode == IRMMOVQ ||
        D_icode == IOPQ    ||
        D_icode == IPUSHQ)
     {
        return D_rA;
     }
     else if(D_icode == IOPQ ||
             D_icode == IRET)
     {
        return RSP;
     }
     
     return RNONE;
 }

/* setSrcB
 * sets the d_srcB component based on the icode
 * and sets dreg to RNONE
 *
 * @param D_icode
 */
uint64_t DecodeStage::getSrcB(uint64_t D_icode, uint64_t D_rB)
{
    if(D_icode == IOPQ    ||
       D_icode == IRMMOVQ ||
       D_icode == IMRMOVQ)
    {
        return D_rB;
    }
    else if (D_icode == IPUSHQ ||
             D_icode == IPOPQ  ||
             D_icode == ICALL  ||
             D_icode == IRET)
    {
        return RSP;
    }
    
    return RNONE;
}

/*setDstE
 * sets the destination register based on the icode
 *
 * @param D_icode
 */
uint64_t DecodeStage::getDstE(uint64_t D_icode, uint64_t D_rB)
{
    if(D_icode == IRRMOVQ ||
       D_icode == IIRMOVQ ||
       D_icode == IOPQ)
    {
        return D_rB;
    }
    else if(D_icode == IPUSHQ ||
            D_icode == IPOPQ  ||
            D_icode == ICALL  ||
            D_icode == IRET)
    {
        return RSP;
    }
    
    return RNONE;
}

/*setDstM
 *sets dstM if D_icode is IMRMOVQ, IPOPQ then returns D_rA
 *
 */
uint64_t DecodeStage::getDstM(uint64_t D_icode, uint64_t D_rA)
{
    if (D_icode == IMRMOVQ ||
        D_icode == IPOPQ)
    {
        return D_rA;
    }
    
    return RNONE;
}

