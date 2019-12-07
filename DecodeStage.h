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
      uint64_t selFwdA(uint64_t d_srcA, M * mreg, E * ereg, W * wreg, Stage ** stages, uint64_t d_rvalA)  ;
      uint64_t FwdB(uint64_t d_srcB, M * mreg, W * wreg, E * ereg, Stage ** stages, uint64_t d_rvalB);
      uint64_t d_srcA;
      uint64_t d_srcB;
      bool E_bubble;
      bool calcE_bubble(uint64_t E_icode, uint64_t E_dstM, uint64_t e_Cnd);
      
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t getd_srcB();
      uint64_t getd_srcA();

};
