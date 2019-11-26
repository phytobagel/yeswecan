#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Memory.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "Tools.h"
#include "W.h"
#include "Stage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "iostream"
#include "Instructions.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
    
   Memory * mem = Memory::getInstance();
   bool error = false; 
   uint64_t icode = 0, ifun = 0, valC = 0, valP = 0;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;

   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.
   
   uint64_t f_pc = selectPC(freg, mreg, wreg);
   uint8_t instructionByte = mem->getByte(f_pc, error);
   icode = Tools::getBits(instructionByte,4,7);
   ifun = Tools::getBits(instructionByte,0,3); 
    
   bool needValC = FetchStage::needValC(icode);
   bool needRegIds = FetchStage::needRegIds(icode);
   
   uint64_t startAddress = f_pc + 1;
   if (needRegIds)
   {   
       getRegIds(startAddress, rA, rB);
       startAddress++;
   }
   if (needValC)
   {
       valC = getValC(startAddress);
   }
   
   valP = PCIncrement(needValC, needRegIds, valP, f_pc);
   uint64_t pred_PC = predictPC(valP, valC, icode);

   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(pred_PC);
   
   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);

   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];

   freg->getpredPC()->normal();
   dreg->getstat()->normal();
   dreg->geticode()->normal();
   dreg->getifun()->normal();
   dreg->getrA()->normal();
   dreg->getrB()->normal();
   dreg->getvalC()->normal();
   dreg->getvalP()->normal();
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
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}

uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{    
    if(mreg->geticode()->getOutput() == IJXX && !mreg->getCnd()->getOutput())
    {
        return mreg->getvalA()->getOutput();
    }
    else if(wreg->geticode()->getOutput() == IRET)
    {
        return wreg->getvalM()->getOutput();
    }
    else
    {
        return freg->getpredPC()->getOutput();
    }
}

bool FetchStage::needRegIds(uint64_t f_icode)
{
    if (f_icode == IRRMOVQ ||
        f_icode == IOPQ    ||
        f_icode == IPUSHQ  ||
        f_icode == IPOPQ   ||
        f_icode == IIRMOVQ ||
        f_icode == IRMMOVQ  ||
        f_icode == IMRMOVQ)
    {
        return true;
    } 
    return false;
}

//getRegIds
void FetchStage::getRegIds(uint64_t f_pc, uint64_t & rA, uint64_t & rB)
{
    bool imem_error = false;
    Memory * mem = Memory::getInstance();
    uint64_t byte = mem->getByte(f_pc, imem_error);
    rA = (uint64_t) Tools::getBits(byte, 4, 7);
    rB = (uint64_t) Tools::getBits(byte, 0, 3);
}

bool FetchStage::needValC(uint64_t f_icode)
{
    if (f_icode == IIRMOVQ ||
        f_icode == IRMMOVQ ||
        f_icode == IMRMOVQ ||
        f_icode == IJXX    ||
        f_icode == ICALL)
    {
        return true;
    } 
    return false;
}


uint64_t FetchStage::getValC(uint64_t f_pc)
{
    bool imem_error = false;
    Memory * mem = Memory::getInstance();
    uint8_t byte[8];
    uint64_t start = f_pc;

    for (int i = 0; i < 8; i++)
    {
        byte[i] = mem->getByte(start + i, imem_error);
    }

    uint64_t f_valC = Tools::buildLong(byte);
    return f_valC;
}

uint64_t FetchStage::predictPC(uint64_t ValP, uint64_t ValC, uint64_t icode)
{
    if (icode == IJXX || icode == ICALL)
    {
        return ValC;
    } 
    
    return ValP; 
}

uint64_t FetchStage::PCIncrement(bool needValC, bool needRegIds, uint64_t & valP, uint64_t f_pc)
{
   if (needRegIds)
    {
        f_pc++;    
    }
   
   if (needValC)
    {
        f_pc += 8;
    }
    f_pc++;
    valP = f_pc;
    return f_pc;
}



     

