#ifndef PROGRAM_CONFIGFORMATEXCEPTION_H
#define PROGRAM_CONFIGFORMATEXCEPTION_H


#include <exception>
#include <string>

class ConfigFormatException : public std::exception {
public:
	explicit ConfigFormatException(std::string message) noexcept;
	const char *what() const noexcept override;

private:
	std::string message;
};


#endif	// PROGRAM_CONFIGFORMATEXCEPTION_H
