#pragma once
#ifndef KEYSIMULATORFUNCTIONS_H
#define KEYSIMULATORFUNCTIONS_H

#include <vector>
#include <Windows.h>

void PressKey(WORD key);
void PressMultipleKeys(const std::vector<WORD>& keys);

#endif