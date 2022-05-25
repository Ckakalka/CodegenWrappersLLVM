#include "TranslatorException.h"

TranslatorException::TranslatorException(std::string message) noexcept : message(std::move(message)) {}

const char *TranslatorException::what() const noexcept { return message.c_str(); }