"""
    @file    decompose.py
    @brief   This script implements a generator of C++ code for type definition at runtime
    @usage   python decompose.py [path to JSON with type info] [path to output C++ header file]
    @author  DronCode
    @license MIT
"""
import sys
import json
import datetime


class TypeRow:
    def __init__(self, index, name, parent=None):
        self._index = index
        self._name = name
        self._parent = parent

    @property
    def PrettyName(self):
        if not self._parent:
            return "{}".format(self._name)
        else:
            return "{}_{}".format(self._name, self._parent)

    @property
    def Index(self):
        return int(self._index, 16)

    @property
    def HexIndex(self):
        return self._index

    @property
    def ClassName(self):
        return self._name

    @property
    def ParentClassName(self):
        return self._parent


def generate_definitions(input_definitions_file, output_cpp_header_file):
    with open(input_definitions_file, "r") as source_definitions_file_handler:
        type_info_file_json = json.load(source_definitions_file_handler)

        with open(output_cpp_header_file, "w") as cpp_header_output_file:
            class_list = []
            used_keys_set = set()

            for type_id in type_info_file_json:
                if not type_info_file_json[type_id][0] in used_keys_set:
                    class_list.append(
                        TypeRow(type_id, type_info_file_json[type_id][0], type_info_file_json[type_id][1]))
                    used_keys_set.add(type_info_file_json[type_id][0])
                else:
                    for copy_index in range(1, 100):
                        if not "{}_{}".format(type_info_file_json[type_id][0], copy_index) in used_keys_set:
                            new_name = "{}_{}".format(type_info_file_json[type_id][0], copy_index)
                            class_list.append(TypeRow(type_id, new_name, type_info_file_json[type_id][1]))
                            used_keys_set.add(new_name)
                            break

            # Add not initialised type id of default initialisation in C++ code
            class_list.append(TypeRow("std::numeric_limits<unsigned int>::max()-1", "NOT_FOUND"))
            class_list.append(TypeRow("std::numeric_limits<unsigned int>::max()", "NOT_INITIALISED"))

            cpp_header_output_file.write("/*\n")
            cpp_header_output_file.write("   THIS IS AUTOGENERATED FILE! DO NOT EDIT!\n")
            cpp_header_output_file.write("   Generated by decompose.py at {}\n*/\n\n".format(datetime.datetime.now()))
            cpp_header_output_file.write("#ifndef __GLACIER_TYPE_IDS_H__\n")
            cpp_header_output_file.write("#define __GLACIER_TYPE_IDS_H__\n")
            cpp_header_output_file.write("#include <limits>\n\n")
            cpp_header_output_file.write("namespace Glacier {\n")
            # Generate enum
            cpp_header_output_file.write("\tenum TypeId : unsigned int {\n")
            get_type_name = lambda t: "\t\t{} = {}".format(t.PrettyName, t.HexIndex)
            cpp_header_output_file.write(',\n'.join(map(get_type_name, class_list)))
            # Print footer
            cpp_header_output_file.write("\n\t};\n\n")
            cpp_header_output_file.write("}\n")
            cpp_header_output_file.write("#endif")
            cpp_header_output_file.close()

        source_definitions_file_handler.close()


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("""
            Usage:
                python decompose.py [glacier types database file path] [header output location]
        """)
    else:
        generate_definitions(sys.argv[1], sys.argv[2])
