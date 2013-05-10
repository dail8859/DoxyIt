// Most of the features of the parser have been tested in test.c
// Now just test c++ specific features

int namespace::Class::Method()

std::string& Class::Method()

std::string &Class::Method()

std::string && Class::Method()

void Class::Method(std::string &str)

void Class::Method(int x, float pi = 3.14)

void Class::Method(std::mystring = std::string())	// This doesn't work, but should

Constructor(int x) // bug if space is before parenthesis

~Destructor()
