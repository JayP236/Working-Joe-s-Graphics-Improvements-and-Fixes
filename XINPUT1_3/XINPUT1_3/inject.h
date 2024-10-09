#pragma once
	
#include <cctype>
#include <iostream>
#include <string>
#include "Windows.h"

void Inject(HINSTANCE__* hModule, unsigned long ulReasonForCall);