'''
    Parse a Cheat Engine structure xml file and convert it to a C structure.
'''

import xml.etree.ElementTree as ET
import re
import os

def parse_structure(structure):
    elements = []
    for element in structure.findall('Elements/Element'):
        offset = int(element.get('Offset'))
        vartype = element.get('Vartype')
        bytesize = int(element.get('Bytesize'))
        description = element.get('Description', '')
        displaymethod = element.get('DisplayMethod', '')
        if not description == '':
            elements.append((offset, vartype, bytesize, description, displaymethod))
    return elements

def format_identifier(description):
    # Split on non-alphanumeric characters. Output camelCase.
    words = re.split(r'\W+', description)
    first = words[0][0].lower() + words[0][1:]
    return first + ''.join([word.capitalize() for word in words[1:]])

def get_c_type(vartype, displaymethod, bytesize):
    isSigned = 'unsigned' not in displaymethod
    if vartype == 'Byte':
        return 'uint8_t' if isSigned else 'int8_t'
    if vartype == '2 Bytes':
        return 'uint16_t' if isSigned else 'int16_t'
    if vartype == '4 Bytes':
        return 'uint32_t' if isSigned else 'int32_t'
    if vartype == '8 Bytes':
        return 'uint64_t' if isSigned else 'int64_t'
    if vartype == 'Float':
        return 'float'
    if vartype == 'Double':
        return 'double'
    return vartype

def get_c_fields(structure):
    elements = parse_structure(structure)
    fields = []

    running_offset = 0
    for element in elements:
        offset, vartype, bytesize, description, displaymethod = element

        # Add padding if necessary.
        if offset > running_offset:
            padding = offset - running_offset
            line = '    uint8_t pad_%04X[%d];' % (offset - padding, padding)
            fields.append((line, running_offset))
            running_offset = offset
        
        identifier = format_identifier(description)
        ctype = get_c_type(vartype, displaymethod, bytesize)
        line = '    %s %s;' % (ctype, identifier)
        fields.append((line, offset))
        
        running_offset += bytesize
        
    return fields

def struct_to_c(structure):
    c_fields = get_c_fields(structure)

    max_line = max([len(line) for line, _ in c_fields])

    c_struct = '#pragma pack(push, 1)\nstruct %s {\n' % structure.get('Name')

    for line, offset in c_fields:
        c_struct += line.ljust(max_line) + ' // %04X\n' % offset
    
    c_struct += '};\n#pragma pack(pop)\n'

    return c_struct

def convert_file(filepath):
    print("Converting file: %s" % filepath)
    tree = ET.parse(filepath)
    root = tree.getroot()
    output = '#include <stdint.h>\n\n'
    for structure in root.findall('Structure'):
        cstruct = struct_to_c(structure)
        output += cstruct + '\n'
        print(cstruct)
    
    filename = os.path.basename(filepath)
    filename = os.path.splitext(filename)[0]
    filename = filename + '.c'
    with open(filename, 'w') as f:
        f.write(output)

    return output

if __name__ == '__main__':
    import sys

    if len(sys.argv) == 2:
        convert_file(sys.argv[1])
    else:
        for filename in os.listdir('.'):
            if filename.endswith('.CSX'):
                convert_file(filename)
