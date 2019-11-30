//class to perform the combinational logic of
//the Fetch stage
class ExecuteStage: public Stage
{
   private:
      void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, 
                     uint64_t valE, uint64_t valA,
                     uint64_t dstE, uint64_t dstM);
      uint64_t aluA(uint64_t E_icode, uint64_t E_valA, uint64_t E_valC);
      uint64_t aluB(uint64_t E_icode, uint64_t E_valB);
      uint64_t alufun(uint64_t E_icode, uint64_t E_ifun);
      bool set_cc(uint64_t E_icode);
      uint64_t e_dstE(uint64_t E_icode, bool e_Cnd, uint64_t E_dstE);
      uint64_t CC(bool set_cc, uint64_t ALUoutput);


   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
