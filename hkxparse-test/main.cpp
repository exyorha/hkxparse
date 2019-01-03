#include <hkxparse/HKXFile.h>
#include <hkxparse/PrettyPrinter.h>

#include <fstream>

int main(int argc, char *argv[]) {
	hkxparse::HKXFile file;
	file.loadFile("C:\\projects\\havok\\skeleton_tag.hkx"); // skeleton_tag

	{
		std::ofstream stream;
		stream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
		stream.open("C:\\projects\\hkxparse\\dump.txt", std::ios::out | std::ios::trunc);

		hkxparse::PrettyPrinter printer(stream);
		printer.print(file.root());
	}

	__debugbreak();
}
