#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "Instructions.h"
#include "W.h"
#include "Stage.h"
#include "WritebackStage.h"
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
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    
    W * wreg = (W *) pregs[WREG];
    uint64_t icode = wreg->geticode()->getOutput(); 
    
    
    if(icode == IHALT)
    {
        return true;
    } //else
    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg ** pregs)
{
   W * wreg = (W *) pregs[WREG];
   RegisterFile * reg = RegisterFile::getInstance();
   bool error = false;   
       
   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.

   uint64_t W_valE = wreg->getvalE()->getOutput();
   uint64_t W_dstE = wreg->getdstE()->getOutput(); 
    
   reg->RegisterFile::writeRegister(W_valE, W_dstE, error);

  
}

