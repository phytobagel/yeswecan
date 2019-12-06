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
      bool set_cc(uint64_t E_icode, Stage ** stages, W * wreg);
      void CC(uint64_t ALUoutput, bool overflow);
      uint64_t ALU(uint64_t ALU_A, uint64_t ALU_B, uint64_t ALU_fun, bool set_cc);
      uint64_t e_dstE(uint64_t E_icode, bool e_Cnd, uint64_t E_dstE);
      uint64_t cond(uint64_t E_icode, uint64_t E_ifun);
      bool calculateControlSignals(Stage ** stages, W * wreg);
      bool M_bubble;

   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
uint64_t e_dstE_var;
      uint64_t e_valE_var;
      uint64_t gete_dstE();
      uint64_t gete_valE();
};
