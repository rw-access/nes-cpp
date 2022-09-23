#pragma once
#include <stdint.h>

extern "C" void *CreateInteractiveConsole(const char *path);
extern "C" void HandleInteraction(void *console);
extern "C" void StepFrame(void *console);
extern "C" void UpdateButtons(void *console, uint8_t buttons);
extern "C" void FrameLimit(void *console);
extern "C" bool Done(void *console);