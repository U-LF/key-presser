#pragma once
#ifndef MACRO_H
#define MACRO_H

#include <vector>
#include <string>
#include <Windows.h>

struct Macro {
    std::string name;
    std::vector<WORD> keys;
    int delay;
};

#endif