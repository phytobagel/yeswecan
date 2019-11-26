//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool needRegIds(uint64_t icode);
      bool needValC(uint64_t icode);
      uint64_t predictPC(uint64_t ValP, uint64_t ValC, uint64_t icode);
      uint64_t PCIncrement(bool needValC, bool needRegIds, uint64_t & valP, uint64_t f_pc);
      void getRegIds(uint64_t f_pc, uint64_t & rA, uint64_t & rB);
      uint64_t getValC(uint64_t f_pc);

      

   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
