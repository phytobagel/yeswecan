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
#include "ExecuteStage.h"
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
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   W * wreg = (W *) pregs[WREG];
   M * mreg = (M *) pregs[MREG];
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];
   uint64_t E_icode = ereg->geticode()->getOutput();
   uint64_t E_dstM = ereg->getdstM()->getOutput();

   RegisterFile * reg = RegisterFile::getInstance();
   bool error = false;

   uint64_t D_icode = dreg->geticode()->getOutput(), 
            D_ifun = dreg->getifun()->getOutput(), 
            D_valC = dreg->getvalC()->getOutput(), 
            d_valA = 0, 
            d_valB = 0;
            //D_valP = dreg->getvalP()->getOutput();
   uint64_t d_dstE = RNONE, d_dstM = RNONE, 
            D_stat = dreg->getstat()->getOutput();

   d_srcA = getSrcA(D_icode,dreg->getrA()->getOutput());
   d_srcB = getSrcB(D_icode,dreg->getrB()->getOutput());
   d_dstE = getDstE(D_icode,dreg->getrB()->getOutput());
   d_dstM = getDstM(D_icode,dreg->getrA()->getOutput());
   
   uint64_t d_rvalA = reg->readRegister(d_srcA,error);
   uint64_t d_rvalB = reg->readRegister(d_srcB,error);

   d_valA = selFwdA(d_srcA, mreg, ereg, wreg, stages, d_rvalA);
   d_valB = FwdB(d_srcB, mreg, wreg, ereg, stages, d_rvalB);

   E_bubble = calcE_bubble(E_icode, E_dstM);
   //provide the input values for the D register
   setEInput(ereg, D_stat, D_icode, D_ifun, D_valC, d_valA, 
             d_valB, d_dstE, d_dstM, d_srcA, d_srcB);
   return false;
}

bool DecodeStage::calcE_bubble(uint64_t E_icode, uint64_t E_dstM)
{
    return (E_icode == IMRMOVQ ||
            E_icode == IPOPQ) &&
           (E_dstM == d_srcA ||
            E_dstM == d_srcB);
}

uint64_t DecodeStage::getd_srcA()
{
    return d_srcA;
}

uint64_t DecodeStage::getd_srcB()
{
    return d_srcB;
}

uint64_t DecodeStage::selFwdA(uint64_t d_srcA, M * mreg, E * ereg, W * wreg, Stage ** stages, uint64_t d_rvalA) 
{
    ExecuteStage * eStage = (ExecuteStage *) stages[ESTAGE];
    MemoryStage * mStage = (MemoryStage *) stages[MSTAGE];

    if (d_srcA == RNONE) return 0;

    if (d_srcA == eStage->gete_dstE())
        return eStage->gete_valE();

    if (d_srcA == mreg->getdstM()->getOutput())
        return mStage->getm_valM();

    if (d_srcA == mreg->getdstE()->getOutput())
        return mreg->getvalE()->getOutput();

    if (d_srcA == wreg->getdstE()->getOutput())
        return wreg->getvalE()->getOutput();

    if (d_srcA == wreg->getdstM()->getOutput())
        return wreg->getvalM()->getOutput();

    return d_rvalA;
}

uint64_t DecodeStage::FwdB(uint64_t d_srcB, M * mreg, W * wreg, E * ereg, Stage ** stages, uint64_t d_rvalB )
{
    ExecuteStage * eStage = (ExecuteStage *) stages[ESTAGE];
    MemoryStage * mStage = (MemoryStage *) stages[MSTAGE];

    if (d_srcB == RNONE) return 0;

    if (d_srcB == eStage->gete_dstE())
        return eStage->gete_valE();

    if (d_srcB == mreg->getdstM()->getOutput())
        return mStage->getm_valM();

    if (d_srcB == mreg->getdstE()->getOutput())
        return mreg->getvalE()->getOutput();

    if (d_srcB == wreg->getdstM()->getOutput())
        return wreg->getvalM()->getOutput();

    if (d_srcB == wreg->getdstE()->getOutput())
        return wreg->getvalE()->getOutput();

    return d_rvalB;
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

   if (!E_bubble)
   {
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
   else
   {
   ereg->getstat()->bubble(SAOK);
   ereg->geticode()->bubble(INOP);
   ereg->getifun()->bubble();
   ereg->getvalC()->bubble();
   ereg->getvalA()->bubble();
   ereg->getvalB()->bubble();
   ereg->getdstE()->bubble(RNONE);
   ereg->getdstM()->bubble(RNONE);
   ereg->getsrcA()->bubble(RNONE);
   ereg->getsrcB()->bubble(RNONE);
   }
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
     else if(D_icode == IPOPQ ||
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

