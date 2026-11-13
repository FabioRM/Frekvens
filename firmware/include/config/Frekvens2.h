#pragma once

#include "secrets.h"

#undef PIN_MIC
#define PIN_MIC 7

#undef PIN_OE
#define PIN_OE 11

#undef PIN_MOSI
#define PIN_MOSI 10

#undef PIN_SCLK
#define PIN_SCLK 9

#undef PIN_CS
#define PIN_CS 8

#undef PIN_SW1
#define PIN_SW1 1

#undef PIN_SW2
#define PIN_SW2 2

#include "config/IkeaFrekvens.h"

#undef MANUFACTURER
#define MANUFACTURER "Waveshare"

#undef MODEL
#define MODEL "Frekvens2"
