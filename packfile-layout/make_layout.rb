#!/usr/bin/env ruby

require 'json'

if ARGV.size != 3
	warn "Usage: make_layout <INPUT JSON FILE> <OUTPUT HEADER FILE> <OUTPUT SOURCE FILE>"
	exit 1
end

input_filename, header_filename, source_filename = ARGV
layout = JSON.parse File.read(input_filename)

layout_name = File.basename(input_filename, ".json").gsub(/[-.]/, "_")

File.open(source_filename, "w") do |outf|
	outf.write <<EOF
/*
 * Automatically generated from #{input_filename}
 */

#include <hkxparse/HavokReflectionTypes.h>
#include <#{File.basename(input_filename, ".json")}.h>

namespace hkxparse {
namespace #{layout_name} {
EOF

	layout["classes"].each do |class_info|
		outf.puts "extern const HavokClass #{class_info["name"]}Class;"
	end

	layout["typeinfo"].each do |typeinfo|
		outf.puts "extern const HavokTypeInfo #{typeinfo["name"]}TypeInfo;"
	end

	layout["classes"].each do |class_info|
		class_name = class_info["name"]

		enums = class_info["enums"]

		if enums.size > 0
			enums.each do |enum|
				items = enum["items"]
				next if items.empty?

				outf.puts "static const HavokClassEnumItem #{class_name}#{enum["name"]}Items[] = {"

				items.each do |item|
					outf.write <<EOF
  {
    #{item["value"]},
	#{item["name"].inspect}
  },
EOF
				end

				outf.puts "};"
			end

			outf.puts "static const HavokClassEnum #{class_name}ClassEnums[] = {"

			enums.each do |enum|
				outf.puts "  {"

				outf.puts "    #{enum["name"].inspect},"

				items = enum["items"]
				if items.size > 0
					outf.puts "    #{class_name}#{enum["name"]}Items,"
				else
					outf.puts "    nullptr,"
				end
				outf.puts "    #{items.size},"

				outf.puts "  },"
			end

			outf.puts "};"
		end

		members = class_info["members"]

		if members.size > 0
			outf.puts "static const HavokClassMember #{class_name}ClassMembers[] = {"

			members.each do |member|
				outf.puts "  {"

				outf.puts "    #{member["name"].inspect},"

				if member["class"].nil?
					outf.puts "    nullptr,"
				else
					outf.puts "    &#{member["class"]}Class,"
				end

				outf.puts "    static_cast<HavokType>(#{member["type"]}),"
				outf.puts "    static_cast<HavokType>(#{member["subtype"]}),"
				outf.puts "    #{member["arraySize"]},"
				outf.puts "    #{member["flags"]},"
				outf.puts "    #{member["offset"]},"

				outf.puts "  },"
			end

			outf.puts "};"
		end

		outf.puts "const HavokClass #{class_name}Class = {"
		outf.puts "  #{class_name.inspect},"

		parent_name = class_info["parent"]
		if parent_name
			outf.puts "  &#{parent_name}Class,"
		else
			outf.puts "  nullptr,"
		end

		outf.puts "  #{class_info["objectSize"]},"

		if enums.size > 0
			outf.puts "  #{class_name}ClassEnums,"
		else
			outf.puts "  nullptr,"
		end
		outf.puts "  #{enums.size},"

		if members.size > 0
			outf.puts "  #{class_name}ClassMembers,"
		else
			outf.puts "  nullptr,"
		end
		outf.puts "  #{members.size},"

		outf.puts "  #{class_info["flags"]},"
		outf.puts "  #{class_info["describedVersion"]},"

		outf.puts "};"
	end

	layout["typeinfo"].each do |typeinfo|
		outf.puts "const HavokTypeInfo #{typeinfo["name"]}TypeInfo = {"
		outf.puts "  #{typeinfo["name"].inspect},"
		outf.puts "  #{typeinfo["vtable"]},"
		outf.puts "};"
	end

	outf.puts "}";

	outf.puts "const HavokClass *const #{layout_name}Classes[] = {"

	layout["classes"].sort_by { |val| val["name"] }.each do |klass|
		outf.puts "  &#{layout_name}::#{klass["name"]}Class,"
	end

	outf.puts "};"
	
	outf.puts "const HavokTypeInfo *const #{layout_name}TypeInfo[] = {"

	layout["typeinfo"].sort_by { |val| val["name"] }.each do |klass|
		outf.puts "  &#{layout_name}::#{klass["name"]}TypeInfo,"
	end

	outf.puts "};"

	outf.puts "}"
end

File.open(header_filename, "w") do |outf|
	outf.write <<EOF
/*
 * Automatically generated from #{input_filename}
 */
#ifndef REFLECTION_#{layout_name}
#define REFLECTION_#{layout_name}

namespace hkxparse {
	struct HavokClass;
	struct HavokTypeInfo;

	extern const HavokClass *const #{layout_name}Classes[#{layout["classes"].size}];
	extern const HavokTypeInfo *const #{layout_name}TypeInfo[#{layout["typeinfo"].size}];
}

#endif
EOF

end
