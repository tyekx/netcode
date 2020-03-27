#pragma once

#include <iostream>
#include <vector>

enum EFormatterAlignment {
	E_CENTER, E_LEFT, E_RIGHT
};

class ConsoleFormatter {

	struct HeaderData {
		std::string Title;
		uint32_t Width;
		EFormatterAlignment Alignment;
		uint32_t Precision;
	};

	std::vector<HeaderData> Headers;
	uint32_t ColumnIterator;
	std::string Separator;
	std::ostream & Stream;

	void AddColumn(const std::string & title, uint32_t prec, uint32_t width, EFormatterAlignment align);


	void PrintCenter(const std::string & text, uint32_t width);
	void PrintLeft(const std::string & text, uint32_t width);
	void PrintRight(const std::string & text, uint32_t width);
	void PrintAligned(const std::string & text, uint32_t width, EFormatterAlignment align);
public:
	ConsoleFormatter(std::ostream & os);

	void BeginColumnDeclaration();
	void AddTextColumn(const std::string & title, uint32_t width = 20, EFormatterAlignment alignment = E_RIGHT);
	void AddNumberColumn(const std::string & title, uint32_t precision = 4, uint32_t width = 20, EFormatterAlignment alignment = E_CENTER);
	void EndColumnDeclaration();

	void BeginRow();
	void PrintValue(double x);
	void PrintValue(int x);
	void PrintValue(const std::string & s);
	void PrintSeparator();
	void PrintHeader();
	void EndRow();
};
