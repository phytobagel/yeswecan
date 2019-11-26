//class to perform the combinational logic of
//the Fetch stage
class DecodeStage: public Stage
{
   private:
      void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t valC, uint64_t valA, uint64_t valB,
                     uint64_t dstE, uint64_t dstM, uint64_t srcA,
                     uint64_t srcB);
      uint64_t getSrcA(uint64_t D_icode, uint64_t D_rA);

      uint64_t getSrcB(uint64_t D_icode, uint64_t D_rB);
      uint64_t getDstE(uint64_t D_icode, uint64_t D_rB);
      uint64_t getDstM(uint64_t D_icode, uint64_t D_rA);


   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
