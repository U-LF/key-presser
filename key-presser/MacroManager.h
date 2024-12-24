#pragma once
#ifndef MACROMANAGER_H
#define MACROMANAGER_H

#include "Macro.h"
#include <vector>

void SaveMacro(const Macro& macro);
std::vector<Macro> LoadMacros();

#endif