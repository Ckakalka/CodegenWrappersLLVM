#include "ConfigFormatException.h"

ConfigFormatException::ConfigFormatException(std::string message) noexcept : message(std::move(message)) {}

const char *ConfigFormatException::what() const noexcept {
	return message.c_str();
}
