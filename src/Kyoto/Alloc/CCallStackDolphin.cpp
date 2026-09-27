#include "Kyoto/Alloc/CCallStack.hpp"

const char CCallStack::kUnknownType[] = "UnknownType\0";

CCallStack::CCallStack(uint lineNum, const char* fileAndLine, const char* type)
: mLine(fileAndLine), mType(type) {}

const char* CCallStack::GetFileAndLineText() const { return mLine; }

const char* CCallStack::GetTypeText() const { return mType; }
