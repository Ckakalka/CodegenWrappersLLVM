
#ifndef WRAPGEN_TRANSLATOREXCEPTION_H
#define WRAPGEN_TRANSLATOREXCEPTION_H


#include <exception>
#include <string>

class TranslatorException : public std::exception {
public:
	explicit TranslatorException(std::string message) noexcept;
	const char *what() const noexcept override;

private:
	std::string message;
};


#endif	// WRAPGEN_TRANSLATOREXCEPTION_H
