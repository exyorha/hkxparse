#include <hkxparse/PrettyPrinter.h>
#include <functional>
#include <string>
#include <algorithm>

namespace hkxparse {
	PrettyPrinter::PrettyPrinter(std::ostream &stream) : m_stream(stream), m_state(State::StartOfLine), m_level(0) {

	}

	PrettyPrinter::~PrettyPrinter() {

	}

	void PrettyPrinter::print(const HKXVariant &value) {
		std::visit([=](auto &&val) {
			doPrint(val);
		}, value);
	}

	void PrettyPrinter::doPrint(std::monostate) {
		printValue("NULL");
	}

	void PrettyPrinter::doPrint(uint64_t val) {
		printValue(std::to_string(static_cast<int64_t>(val)).c_str());
	}

	void PrettyPrinter::doPrint(const HKXStruct &dictionary) {
		printValueNoNewLine("STRUCT:");

		bool first = true;

		for (const auto &type : dictionary.classNames) {
			if (first) {
				first = false;
			}
			else {
				printValueNoNewLine(" -> ");
			}
			printValueNoNewLine(type);
		}

		printValue("");

		increaseLevel();

		for (const auto &pair : dictionary.fields) {
			printKey(pair.first);
			print(pair.second);
		}

		decreaseLevel();
	}

	void PrettyPrinter::doPrint(const HKXArray &ary) {
		printValue("ARRAY");

		increaseLevel();

		for (const auto &item : ary.values) {
			print(item);
		}

		decreaseLevel();
	}

	void PrettyPrinter::doPrint(const std::vector<unsigned char> &byteArray) {
		printValue("BYTEARRAY");

		increaseLevel();

		for (size_t offset = 0; offset < byteArray.size(); offset += 16) {
			char buf[16];
			snprintf(buf, sizeof(buf), "%04zX ", offset);

			printValueNoNewLine(buf);

			auto chunk = std::min<size_t>(byteArray.size() - offset, 16);

			for (size_t byte = 0; byte < chunk; byte++) {
				snprintf(buf, sizeof(buf), "%02X ", byteArray[offset + byte]);
				printValueNoNewLine(buf);
			}

			for (size_t byte = chunk; byte < 16; byte++) {
				printValueNoNewLine("   ");
			}

			printValueNoNewLine(" | ");

			for (size_t byte = 0; byte < chunk; byte++) {
				auto ch = byteArray[offset + byte];
				snprintf(buf, sizeof(buf), "%c", (ch >= 0x20 && ch < 0x7F) ? ch : '.');
				printValueNoNewLine(buf);
			}

			printValue("");
		}

		decreaseLevel();
	}

	void PrettyPrinter::doPrint(const std::string &string) {
		printValueNoNewLine("\"");
		printValueNoNewLine(string.c_str());
		printValue("\"");
	}

	void PrettyPrinter::doPrint(const HKXStructRef &ref) {
		printValueNoNewLine("REF:");
		printValue(std::to_string(reinterpret_cast<uintptr_t>(&*ref)).c_str());

		if (ref) {
			printReference(ref);
		}
	}
	
	void PrettyPrinter::printReference(const HKXStructRef &ref) {
		auto result = m_referencesPrinted.emplace(ref.get());

		if (result.second) {
			increaseLevel();

			print(*ref);

			decreaseLevel();
		}
	}

	void PrettyPrinter::doPrint(float val) {
		printValue(std::to_string(val).c_str());
	}


	void PrettyPrinter::doPrint(const HKXVector4 &val) {
		printValueNoNewLine("(");
		printValueNoNewLine(std::to_string(val.x).c_str());
		printValueNoNewLine(" ");
		printValueNoNewLine(std::to_string(val.y).c_str());
		printValueNoNewLine(" ");
		printValueNoNewLine(std::to_string(val.z).c_str());
		printValueNoNewLine(" ");
		printValueNoNewLine(std::to_string(val.w).c_str());
		printValue(")");

	}
	void PrettyPrinter::doPrint(const HKXQuaternion &val) {
		printValueNoNewLine("Quaternion");
		doPrint(val.vec);
	}

	void PrettyPrinter::doPrint(const HKXMatrix3 &val) {
		printValue("Matrix3");

		increaseLevel();

		doPrint(val.v[0]);
		doPrint(val.v[1]);
		doPrint(val.v[2]);

		decreaseLevel();
	}

	void PrettyPrinter::doPrint(const HKXQsTransform &val) {
		printValue("QsTransform");

		increaseLevel();
		printKey("Translation");
		doPrint(val.translation);
		printKey("Rotation");
		doPrint(val.rotation);
		printKey("Scale");
		doPrint(val.scale);
		decreaseLevel();
	}

	void PrettyPrinter::doPrint(const HKXMatrix4 &val) {
		printValue("Matrix4");

		increaseLevel();

		doPrint(val.v[0]);
		doPrint(val.v[1]);
		doPrint(val.v[2]);
		doPrint(val.v[3]);

		decreaseLevel();
	}

	void PrettyPrinter::startLine() {
		if (m_state == State::StartOfLine) {
			for (size_t level = 0; level < m_level; level++) {
				m_stream << "  ";
			}

			m_state = State::InLine;
		}
	}

	void PrettyPrinter::endLine() {
		if (m_state == State::InLine) {
			m_stream << "\n";
			m_state = State::StartOfLine;
		}
	}

	void PrettyPrinter::printKey(const char *key) {
		startLine();

		m_stream << key;
		m_stream << " = ";
	}

	void PrettyPrinter::printValue(const char *value) {
		startLine();

		m_stream << value;

		endLine();
	}

	void PrettyPrinter::printValueNoNewLine(const char *value) {
		startLine();

		m_stream << value;
	}

	void PrettyPrinter::increaseLevel() {
		m_level++;
	}

	void PrettyPrinter::decreaseLevel() {
		m_level--;
	}
}
