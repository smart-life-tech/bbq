#include <Arduino.h>
#define LIQUID_LEVEL_PIN 36
#define PT100_PIN 23
#define PT100_MULTI_SAMP_CNT 1

void init_liquid_level();
int get_liquid_level_value();

// void init_pt100();
// int get_pt100_value();

String getQueryParams(String header,String key,String endKey);
