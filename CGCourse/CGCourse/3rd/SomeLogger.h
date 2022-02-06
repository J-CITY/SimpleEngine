#ifndef SOMELOGGER_H_INCLUDED
#define SOMELOGGER_H_INCLUDED
//#pragma one
#include <map>
#include <string>
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else

#endif
#include<stdarg.h>

#define PRINT_LINE __LINE__
#define PRINT_LINE_STR std::to_string(__LINE__)

namespace SomeLogger {

enum class LoggerLevel {
	NONE = 0,
	ERR,
	WARNING,
	DEBUG,
	INFO
};
enum class Color {
	Black = 0,
	Blue,
	Green,
	Red,
	Grey,
	LightGrey,
	Cyan,
	Purple,
	LightBlue,
	LightGreen,
	LightCyan,
	LightRed,
	LightPurple,
	Orange,
	Yellow,
	White
};

#ifdef _WIN32
#else
std::map<Color, std::string> colorToFgColor {
	{ Color::Black, "30" },
	{ Color::Grey, "1;30" },
	{ Color::LightGrey, "1;37" },
	{ Color::White, "37" },
	{ Color::Blue, "34"} ,
	{ Color::Green, "32" },
	{ Color::Cyan, "36" },
	{ Color::Red, "31" },
	{ Color::Purple, "35" },
	{ Color::LightBlue, "1;34" },
	{ Color::LightGreen, "1;32" },
	{ Color::LightCyan, "1;36" },
	{ Color::LightRed, "1;31" },
	{ Color::LightPurple, "1;35" },
	{ Color::Orange, "1;33" },
	{ Color::Yellow, "33" }
};

std::map<Color, std::string> colorToBgColor {
	{ Color::Black, "40" },
	{ Color::Grey, "1;40" },
	{ Color::LightGrey, "1;47" },
	{ Color::White, "47" },
	{ Color::Blue, "44"} ,
	{ Color::Green, "42" },
	{ Color::Cyan, "46" },
	{ Color::Red, "41" },
	{ Color::Purple, "45" },
	{ Color::LightBlue, "1;44" },
	{ Color::LightGreen, "1;42" },
	{ Color::LightCyan, "1;46" },
	{ Color::LightRed, "1;41" },
	{ Color::LightPurple, "1;45" },
	{ Color::Orange, "1;43" },
	{ Color::Yellow, "43" }
};
#endif

class String {
	std::string data = "";
public:
	String() {}

	String(std::string in) {
		data = in;
	}
	String(int in) {
		data = std::to_string(in);
	}
	String(long in) {
		data = std::to_string(in);
	}
	String(long long in) {
		data = std::to_string(in);
	}
	String(unsigned in) {
		data = std::to_string(in);
	}
	String(unsigned long in) {
		data = std::to_string(in);
	}
	String(unsigned long long in) {
		data = std::to_string(in);
	}
	String(float in) {
		data = std::to_string(in);
	}
	String(double in) {
		data = std::to_string(in);
	}
	String(long double in) {
		data = std::to_string(in);
	}
	String(bool in) {
		data = in ? "true" : "false";
	}
	String(char in) {
		data = std::string(1, in);
	}
	String(unsigned char in) {
		data = std::string(1, in);
	}

	std::string asStr() const {
		return data;
	}
};


class Logger {
public:
	static Logger& Instance() {
			static Logger logger;
			return logger;
	}

	std::map<LoggerLevel, std::string> levelLabel {
		{ LoggerLevel::NONE, "" },
		{ LoggerLevel::ERR, "ERROR " },
		{ LoggerLevel::WARNING, "WARNING " },
		{ LoggerLevel::DEBUG, "DEBUG " },
		{ LoggerLevel::INFO, "INFO " }
	};

	void setLabelForLevel(LoggerLevel level, std::string label) {
		levelLabel[level] = label;
	}

	Logger& operator<<(const std::string& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const int& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const long& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const long long& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const unsigned& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const unsigned long& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const unsigned long long& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const float& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const double& msg) {
		out(String(msg));
		return *this;
	}
	Logger& operator<<(const long double& msg) {
		out(String(msg));
		return *this;
	}

	Logger& log(LoggerLevel level=LoggerLevel::NONE,
				Color fgColor=Color::White,
				Color bgColor=Color::Black) {
		this->fgColor = fgColor;
		this->bgColor = bgColor;
		this->level = level;
		return *this;
	}

    Logger& printEndl(bool isEndl) {
        this->isEndl = isEndl;
        return *this;
    }

	void logFormat(char* format, ...) {
	    std::string allFormat = format;
		#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, (static_cast<int>(fgColor) + (static_cast<int>(bgColor)<<1)));
		#else
		puts((std::string("\033[") + colorToFgColor[fgColor] + std::string(";") + colorToBgColor[bgColor] + std::string("m")).c_str());
		#endif
		const char *traverse;
		int i;
		char *s;
		allFormat = levelLabel[level] + allFormat;

		va_list arg;
		va_start(arg, allFormat.c_str());
        bool needCheck = true;
		for (auto& traverse : allFormat) {
		    if (traverse == '\0') {
                break;
		    }
			if (traverse != '%' && needCheck) {
                putchar(traverse);
                continue;
			} else if (needCheck) {
			    needCheck = false;
                continue;
			}
            needCheck = true;
			switch (traverse) {
				case 'c' : i = va_arg(arg, int);	//Fetch char argument
							putchar(i);
							break;

				case 'd' : i = va_arg(arg, int); //Fetch Decimal/Integer argument
							if (i < 0) {
								i = -i;
								putchar('-');
							}
							puts(convert(i,10));
							break;

				case 'o': i = va_arg(arg, unsigned int); //Fetch Octal representation
							puts(convert(i, 8));
							break;

				case 's': s = va_arg(arg, char *); //Fetch string
							puts(s);
							break;

				case 'x': i = va_arg(arg, unsigned int); //Fetch Hexadecimal representation
							puts(convert(i, 16));
							break;
				case 'b': i = va_arg(arg,unsigned int); //Fetch binary representation
							puts(convert(i, 2));
							break;
			}
		}
		if (isEndl) {
            putchar('\n');
		}
		va_end(arg);
		#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, 0x0f);
		#else
		puts("\033[0m\n");
		#endif
	}

	~Logger() {
	}
private:
    bool isEndl = false;
	Color fgColor = Color::White;
	Color bgColor = Color::Black;
	LoggerLevel level = LoggerLevel::NONE;
	#ifdef _WIN32
	HANDLE  hConsole;
	#endif

	Logger() {
		#ifdef _WIN32
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		#else

		#endif
	}

	char* convert(unsigned int num, int base) {
		static char Representation[]= "0123456789ABCDEF";
		static char buffer[50];
		char *ptr;

		ptr = &buffer[49];
		*ptr = '\0';

		do {
			*--ptr = Representation[num % base];
			num /= base;
		} while(num != 0);
		return(ptr);
	}

	void out(const String& msg) {
	    #ifdef _WIN32
		SetConsoleTextAttribute(hConsole, (static_cast<int>(fgColor) + (static_cast<int>(bgColor)<<1)));
		std::cout <<levelLabel[level] << msg.asStr() << (isEndl ? "\n" : "");
		SetConsoleTextAttribute(hConsole, 0x0f);
		#else
		std::cout << std::string("\033[") + colorToFgColor[fgColor] + std::string(";") + colorToBgColor[bgColor] +
            std::string("m") + levelLabel[level] + msg + std::string("\033[0m\n") + (isEndl ? "\n" : "");
		#endif
	}
};


class ListElem {
public:
	LoggerLevel level = LoggerLevel::NONE;
	Color fgColor = Color::White;
	Color bgColor = Color::Black;
	std::string data = "";

	ListElem(std::string data="",
			LoggerLevel level=LoggerLevel::NONE,
			Color fgColor=Color::White,
			Color bgColor=Color::Black) {
		this->data = data;
		this->level = level;
		this->fgColor = fgColor;
		this->bgColor = bgColor;
	}

	int size() {
		return data.size();
	}

	void toDefault() {
		level=LoggerLevel::NONE,
		fgColor=Color::White,
		bgColor=Color::Black,
		data = "";
	}
};

class List {
public:
	List() {}

	//TODO add ani simple args
	List& operator<<(const std::string& msg) {
		_list.push_back(ListElem(msg));
		return *this;
	}

	//counr from 0
	void remove(unsigned int id) {
		if (id >= _list.size()) {
			Logger l = Logger::Instance();
			l.log(LoggerLevel::ERR, Color::Red) << "BAD ID";
			return;
		}
		_list.erase(_list.begin() + id);
	}

	void insert(unsigned int id, std::string value) {
		if (id >= _list.size()) {
			Logger l = Logger::Instance();
			l.log(LoggerLevel::ERR, Color::Red) << "BAD ID";
			return;
		}
		_list.insert(_list.begin() + id, ListElem(value));
	}

	ListElem get(unsigned int id) {
		if (id >= _list.size()) {
			Logger l = Logger::Instance();
			l.log(LoggerLevel::ERR, Color::Red) << "BAD ID";
			return ListElem();
		}
		return _list[id];
	}

	void clear() {
		_list.clear();
	}

	void print() {
		Logger l = Logger::Instance();

		auto cnt = 1;
		for (auto& e : _list) {
			l.log(e.level, e.fgColor, e.bgColor) << format(cnt) << separator << e.data << "\n";
			cnt++;
		}
	}

	void setSeparator(std::string sep) {
		separator = sep;
	}

private:
	std::vector<ListElem> _list;
	std::string separator = ". ";

	std::string format(int e) {
		auto elemLen = std::to_string(_list.size() + 1).size();
		auto es = std::to_string(e);
		while (es.size() < elemLen) {
			es += " ";
		}
		return es;
	}
};

class TableElem {
public:
	LoggerLevel level = LoggerLevel::NONE;
	Color fgColor = Color::White;
	Color bgColor = Color::Black;
	std::string data = "";

	TableElem(std::string data="",
			LoggerLevel level=LoggerLevel::NONE,
			Color fgColor=Color::White,
			Color bgColor=Color::Black) {
		this->data = data;
		this->level = level;
		this->fgColor = fgColor;
		this->bgColor = bgColor;
	}

	int size() {
		return data.size();
	}

	void toDefault() {
		level=LoggerLevel::NONE,
		fgColor=Color::White,
		bgColor=Color::Black,
		data = "";
	}
};

class Table {
public:
	Table(int w=0, int h=0) {
		for (auto i = 0; i < h; i++) {
			_table.push_back(std::vector<TableElem>());
			for (auto j = 0; j < w; ++j) {
				_table[i].push_back(TableElem());
			}
		}
	}

	Color fgBorder = Color::Red;
	Color bgBorder = Color::Blue;

	void setElementWidth(int w) {
		elemW = w;
	}

	void insert(unsigned int col, unsigned int row, std::string value,
				LoggerLevel level=LoggerLevel::NONE,
				Color fgColor=Color::White,
				Color bgColor=Color::Black) {
		if (row < _table.size() && col < _table[row].size()) {
			_table[row][col] = TableElem(value, level, fgColor, bgColor);
		}
	}

	void remove(unsigned int col, unsigned int row, std::string value) {
		if (row >= _table.size() || col >= _table[row].size()) {
			Logger l = Logger::Instance();
			l.log(LoggerLevel::ERR, Color::Red) << "BAD IDS";
			return;
		}
		_table[row][col].toDefault();
	}

	std::vector<TableElem> &operator[] (int i) { return _table[i]; }

	void removeRow(int row);
	void removeCol(int col);
	void addRow();
	void addCol();
	void addRowById();
	void addColById();

	std::string get(unsigned int col, unsigned int row) {
		if (row >= _table.size() || col >= _table[row].size()) {
			Logger l = Logger::Instance();
			l.log(LoggerLevel::ERR, Color::Red) << "BAD IDS";
			return "";
		}
		return _table[row][col].data;

	}

	void print() {
		if (_table.size() == 0) {
			return;
		}
		auto colSize = _table[0].size();

		if (elemW < 0) {
			getColSizes();
		}

		auto borderLen = 1;
		if (colSizes.size() > 0) {
			for (auto& e : colSizes) {
				borderLen += e + 1;
			}
		} else {
			borderLen += (elemW + 1) * colSize;
		}
		Logger l = Logger::Instance();
		for (auto i = 0; i < borderLen; ++i) {
			if (i == 0) {
				l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[0];
				continue;
			}
			if (i == borderLen-1) {
				l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[2];
				continue;
			}
			l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[1];
		}
		l.log(LoggerLevel::NONE, fgBorder, bgBorder) << "\n";
		for (auto i = 0; i < _table.size(); i++) {
			l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[3];
			for (auto j = 0; j < _table[i].size(); ++j) {
				TableElem elem = _table[i][j];
				auto elemLen = elemW;
				if (colSizes.size() > 0) {
					elemLen = colSizes[j];
				}
				l.log(elem.level, elem.fgColor, elem.bgColor) << format(elem.data, elemLen);
				l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[3];
			}
			l.log(LoggerLevel::NONE, fgBorder, bgBorder) << "\n";
			if (i == _table.size()-1) {
				continue;
			}
			for (auto k = 0; k < borderLen; ++k) {
				if (k == 0) {
					l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[3];
					continue;
				}
				if (k == borderLen-1) {
					l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[3];
					continue;
				}
				l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[1];
			}
			l.log(LoggerLevel::NONE, fgBorder, bgBorder) << "\n";
		}

		for (auto i = 0; i < borderLen; ++i) {
			if (i == 0) {
				l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[4];
				continue;
			}
			if (i == borderLen-1) {
				l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[6];
				continue;
			}
			l.log(LoggerLevel::NONE, fgBorder, bgBorder) << border[5];
		}
		l.log(LoggerLevel::NONE, fgBorder, bgBorder) << "\n";
	}

private:
	std::vector<std::string> border = {"/", "-", "\\", "|", "\\", "_", "/"};
	std::vector<int> colSizes;

	int elemW = -1;
	std::vector<std::vector<TableElem>> _table;

	std::string format(std::string elem, int size) {
		if (elem.size() >= size) {
			return elem.substr(0, size);
		}

		std::string res = elem;
		auto isFront = false;
		while (res.size() < size) {
			if (isFront) {
				res = " " + res;
			} else {
				res += " ";
			}

			isFront = !isFront;
		}
		return res;
	}

	void getColSizes() {
		colSizes.clear();

		if (!_table.size()) {
			return;
		}
		auto w = _table[0].size();
		for (auto i = 0; i < w; i++) {
			auto _max = 1;
			for (auto j = 0; j < _table.size(); ++j) {
				auto elemLen = _table[j][i].data.size();
				if (elemLen > _max) {
					_max = elemLen;
				}
			}
			colSizes.push_back(_max);
		}
		return;
	}
};


//TODO Custom string
};

#endif // SOMELOGGER_H_INCLUDED