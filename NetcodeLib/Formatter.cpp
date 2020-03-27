#include "Formatter.h"

#include <iomanip>
#include <sstream>


void ConsoleFormatter::AddColumn(const std::string & title, uint32_t prec, uint32_t width, EFormatterAlignment align) {
	HeaderData header;
	header.Title = title;
	if(title.size() > width) {
		header.Title = title.substr(0, width);
	}
	header.Width = width;
	header.Alignment = align;
	header.Precision = prec;
	Headers.push_back(header);
}

void ConsoleFormatter::PrintCenter(const std::string & text, uint32_t width) {
	uint32_t leftW = (width + (uint32_t)text.size()) / 2;
	uint32_t rightW = width - leftW;
	if(text.size() < width) {
		Stream << std::setfill(' ') << std::setw(leftW) << text << std::setfill(' ') << std::setw(rightW) << ' ';
	} else {
		Stream << text;
	}
}

void ConsoleFormatter::PrintLeft(const std::string & text, uint32_t width) {
	if(text.size() == width) {
		Stream << text;
	} else {
		Stream << text << std::setw(width - text.size()) << std::setfill(' ') << ' ';
	}
}

void ConsoleFormatter::PrintRight(const std::string & text, uint32_t width) {
	if(text.size() == width) {
		Stream << text;
	} else {
		std::string p = text + ' ';
		Stream << std::setw(width) << std::setfill(' ') << p;
	}
}

void ConsoleFormatter::PrintAligned(const std::string & text, uint32_t width, EFormatterAlignment align) {
	switch(align) {
		case E_CENTER: PrintCenter(text, width); break;
		case E_LEFT: PrintLeft(text, width); break;
		case E_RIGHT: PrintRight(text, width); break;
	}
}

ConsoleFormatter::ConsoleFormatter(std::ostream & os) : Headers{}, ColumnIterator{ 0 }, Separator{}, Stream{ os } { }

void ConsoleFormatter::BeginColumnDeclaration() {

}

void ConsoleFormatter::AddTextColumn(const std::string & title, uint32_t width , EFormatterAlignment alignment ) {
	AddColumn(title, 0, width, alignment);
}

void ConsoleFormatter::AddNumberColumn(const std::string & title, uint32_t precision , uint32_t width, EFormatterAlignment alignment) {
	AddColumn(title, precision, width, alignment);
}

void ConsoleFormatter::EndColumnDeclaration() {
	std::ostringstream oss;
	oss << "+";
	for(HeaderData & i : Headers) {
		oss << std::setw(i.Width) << std::setfill('-') << '-' << '+';
	}
	Separator = oss.str();
}

void ConsoleFormatter::BeginRow() {
	Stream << '|';
	ColumnIterator = 0;
}

void ConsoleFormatter::EndRow() {
	Stream << std::endl;
}

void ConsoleFormatter::PrintValue(double x) {
	std::ostringstream oss;
	HeaderData & header = Headers[ColumnIterator];
	oss << std::fixed << std::setprecision(header.Precision) << x;
	std::string text = oss.str();
	if(text.size() > header.Width) {
		text = text.substr(0, header.Width);
	}
	PrintAligned(text, header.Width, header.Alignment);
	Stream << '|';
	ColumnIterator++;
}

void ConsoleFormatter::PrintValue(int x) {
	std::ostringstream oss;
	HeaderData & header = Headers[ColumnIterator];
	oss << std::fixed << x;
	std::string text = oss.str();
	if(text.size() > header.Width) {
		text = text.substr(0, header.Width);
	}
	PrintAligned(text, header.Width, header.Alignment);
	Stream << '|';
	ColumnIterator++;
}

void ConsoleFormatter::PrintValue(const std::string & s) {
	HeaderData & header = Headers[ColumnIterator];
	std::string text = s;
	if(text.size() > header.Width) {
		text = text.substr(0, header.Width);
	}
	PrintAligned(text, header.Width, header.Alignment);
	Stream << '|';
	ColumnIterator++;
}

void ConsoleFormatter::PrintSeparator() {
	Stream << Separator << std::endl;
}

void ConsoleFormatter::PrintHeader() {
	PrintSeparator();
	Stream << '|';
	for(HeaderData & i : Headers) {
		PrintCenter(i.Title, i.Width);
		Stream << '|';
	}
	Stream << std::endl;
	PrintSeparator();
}
