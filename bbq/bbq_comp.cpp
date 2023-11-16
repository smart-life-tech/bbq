#include "bbq_comp.h"
void init_liquid_level()
{
  pinMode(LIQUID_LEVEL_PIN, INPUT);
}


// void init_pt100()
// {
//   pinMode(LIQUID_LEVEL_PIN, INPUT);
// }

// int get_pt100_value()
// {
//   int pt100_value = 0;
//   for(int i=0; i<PT100_MULTI_SAMP_CNT; i++) {
//     pt100_value += analogRead(PT100_PIN);
//   }
//   pt100_value /= PT100_MULTI_SAMP_CNT;

//   int voltage = 3300*pt100_value/4095;
//   return voltage;

// }

String getQueryParams(String header, String key, String endKey)
{
  int keyIndex = header.indexOf(key) + key.length();
  int keyEndIndex = header.indexOf(endKey, keyIndex);
  String value = header.substring(keyIndex, keyEndIndex);
  return value;
}