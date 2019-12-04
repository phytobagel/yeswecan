//class to perform the combinational logic of
//the Fetch stage
class MemoryStage: public Stage
{
   private:
      void setWInput(W * wreg, uint64_t stat, uint64_t icode,
                                      uint64_t valE, uint64_t valM,
                                      uint64_t dstE, uint64_t dstM);
      bool mem_read(uint64_t M_icode);
      bool mem_write(uint64_t M_icode);
      uint64_t mem_addr(uint64_t M_icode, uint64_t M_valE, uint64_t M_valA);

   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
