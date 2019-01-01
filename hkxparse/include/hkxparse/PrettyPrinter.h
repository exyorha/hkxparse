#ifndef HKXPARSE_PRETTY_PRINTER_H
#define HKXPARSE_PRETTY_PRINTER_H

#include <iostream>
#include <hkxparse/HKXTypes.h>
#include <unordered_set>

namespace hkxparse {
	class PrettyPrinter {
	public:
		PrettyPrinter(std::ostream &stream);
		~PrettyPrinter();

		PrettyPrinter(const PrettyPrinter &other) = delete;
		PrettyPrinter &operator =(const PrettyPrinter &other) = delete;

		void print(const HKXVariant &value);

	private:
		void doPrint(std::monostate);
		void doPrint(uint64_t val);
		void doPrint(float val);
		void doPrint(const HKXVector4 &val);
		void doPrint(const HKXQuaternion &val);
		void doPrint(const HKXMatrix3 &val);
		void doPrint(const HKXQsTransform &val);
		void doPrint(const HKXMatrix4 &val);
		void doPrint(const HKXStructRef &val);
		void doPrint(const HKXArray &val);
		void doPrint(const std::string &val);
		void doPrint(const HKXStruct &val);
		void doPrint(const std::vector<unsigned char> &val);
		
		void printKey(const char *key);
		void printValue(const char *value);
		void printValueNoNewLine(const char *value);
		void increaseLevel();
		void decreaseLevel();
		void startLine();
		void endLine();

		void printReference(const HKXStructRef &ref);

	private:
		enum class State {
			StartOfLine,
			InLine
		};

		std::ostream &m_stream;
		State m_state;
		size_t m_level;
		std::unordered_set<HKXStruct *> m_referencesPrinted;
	};
}

#endif
