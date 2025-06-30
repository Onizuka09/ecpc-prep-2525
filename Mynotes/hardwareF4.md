- internal btn PA0 
- PD13 : orange LED 
- PD12 : green led 
- PD14 : red led 
- PD15 : blue LED 
## TEmp 
#define AVG_SLOPE (4.3F)
#define V_AT_25C  (1.43F)
#define V_REF_INT (1.2F)
          V_Ref = (float)((V_REF_INT * 4095.0)/AD_RES[0]);
          V_Sense = (float)(AD_RES[1] * V_Ref) / 4095.0;
          Temperature = (((V_AT_25C - V_Sense) * 1000.0) /AVG_SLOPE) + 25.0;
