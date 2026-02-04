#!/usr/bin/env python3
"""
Convert CSV vertex data to GLTF model.
Vertex format: position (X,Y,Z), color (R,G,B), normal (X,Y,Z), UV (U,V)
"""

import csv
import json
import struct
import base64
import sys
from pathlib import Path


class GLTFBuilder:
    def __init__(self):
        self.vertices = []
        self.buffer_data = b''
        self.accessors = []
        self.buffer_views = []
        
    def load_csv_vertices(self, csv_file):
        """Load vertex data from CSV file."""
        print(f"Loading vertex data from {csv_file}...")
        
        with open(csv_file, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line:
                    continue
                    
                # Split by comma and strip whitespace, removing empty strings
                values = [v.strip() for v in line.split(',') if v.strip()]
                
                if len(values) < 10:
                    print(f"Warning: Line {line_num} has {len(values)} values, need at least 10. Skipping.")
                    continue
                    
                try:
                    # Parse vertex data - take first 10 values and use as format: pos(3), color(3), normal(3), uv(2)
                    vertex = {
                        'position': [float(values[0]), float(values[1]), float(values[2])],
                        'color': [float(values[3]), float(values[4]), float(values[5])],
                        'normal': [float(values[6]), float(values[7]), float(values[8])],
                        'uv': [float(values[9]), float(values[10]) if len(values) > 10 else float(values[9])]
                    }
                    self.vertices.append(vertex)
                except ValueError as e:
                    print(f"Warning: Line {line_num} has invalid values: {e}. Skipping.")
                    continue
                except IndexError as e:
                    print(f"Warning: Line {line_num} missing values: {e}. Skipping.")
                    continue
        
        # # Compute the average vertex position for centering
        # if self.vertices:
        #     avg_pos = [0.0, 0.0, 0.0]
        #     for vertex in self.vertices:
        #         for i in range(3):
        #             avg_pos[i] += vertex['position'][i]
        #     num_vertices = len(self.vertices)
        #     avg_pos = [coord / num_vertices for coord in avg_pos]
            
        #     # Center vertices
        #     for vertex in self.vertices:
        #         for i in range(3):
        #             vertex['position'][i] -= avg_pos[i]
        
        print(f"Loaded {len(self.vertices)} vertices")
        
    def create_buffer_view(self, byte_offset, byte_length, target=None):
        """Create a buffer view."""
        buffer_view = {
            "buffer": 0,
            "byteOffset": byte_offset,
            "byteLength": byte_length
        }
        if target is not None:
            buffer_view["target"] = target
            
        self.buffer_views.append(buffer_view)
        return len(self.buffer_views) - 1
        
    def create_accessor(self, buffer_view, component_type, count, type_name, min_vals=None, max_vals=None):
        """Create an accessor."""
        accessor = {
            "bufferView": buffer_view,
            "componentType": component_type,
            "count": count,
            "type": type_name
        }
        
        if min_vals is not None:
            accessor["min"] = min_vals
        if max_vals is not None:
            accessor["max"] = max_vals
            
        self.accessors.append(accessor)
        return len(self.accessors) - 1
        
    def pack_vertex_data(self):
        """Pack vertex data into binary buffer."""
        if not self.vertices:
            raise ValueError("No vertices loaded")
            
        # Calculate bounds
        positions = [v['position'] for v in self.vertices]
        min_pos = [min(p[i] for p in positions) for i in range(3)]
        max_pos = [max(p[i] for p in positions) for i in range(3)]
        
        # Pack positions
        position_data = b''
        for vertex in self.vertices:
            position_data += struct.pack('<fff', *vertex['position'])
            
        position_offset = len(self.buffer_data)
        self.buffer_data += position_data
        position_view = self.create_buffer_view(position_offset, len(position_data), 34962)  # ARRAY_BUFFER
        position_accessor = self.create_accessor(position_view, 5126, len(self.vertices), "VEC3", min_pos, max_pos)  # FLOAT
        
        # Pack colors
        color_data = b''
        for vertex in self.vertices:
            color_data += struct.pack('<fff', *vertex['color'])
            
        color_offset = len(self.buffer_data)
        self.buffer_data += color_data
        color_view = self.create_buffer_view(color_offset, len(color_data), 34962)
        color_accessor = self.create_accessor(color_view, 5126, len(self.vertices), "VEC3")
        
        # Pack normals
        normal_data = b''
        for vertex in self.vertices:
            normal_data += struct.pack('<fff', *vertex['normal'])
            
        normal_offset = len(self.buffer_data)
        self.buffer_data += normal_data
        normal_view = self.create_buffer_view(normal_offset, len(normal_data), 34962)
        normal_accessor = self.create_accessor(normal_view, 5126, len(self.vertices), "VEC3")
        
        # Pack UVs
        uv_data = b''
        for vertex in self.vertices:
            uv_data += struct.pack('<ff', *vertex['uv'])
            
        uv_offset = len(self.buffer_data)
        self.buffer_data += uv_data
        uv_view = self.create_buffer_view(uv_offset, len(uv_data), 34962)
        uv_accessor = self.create_accessor(uv_view, 5126, len(self.vertices), "VEC2")
        
        return position_accessor, color_accessor, normal_accessor, uv_accessor
        
    def generate_indices(self):
        """Generate triangle indices (assuming vertices are already in triangle order)."""
        indices = list(range(len(self.vertices)))
        
        # Pack indices as 16-bit or 32-bit depending on vertex count
        if len(self.vertices) <= 65535:
            index_data = struct.pack(f'<{len(indices)}H', *indices)
            component_type = 5123  # UNSIGNED_SHORT
        else:
            index_data = struct.pack(f'<{len(indices)}I', *indices)
            component_type = 5125  # UNSIGNED_INT
            
        index_offset = len(self.buffer_data)
        self.buffer_data += index_data
        index_view = self.create_buffer_view(index_offset, len(index_data), 34963)  # ELEMENT_ARRAY_BUFFER
        index_accessor = self.create_accessor(index_view, component_type, len(indices), "SCALAR")
        
        return index_accessor
        
    def build_gltf(self):
        """Build the complete GLTF structure."""
        # Pack vertex data
        pos_acc, col_acc, norm_acc, uv_acc = self.pack_vertex_data()
        index_acc = self.generate_indices()
        
        # Encode buffer as base64
        buffer_uri = "data:application/octet-stream;base64," + base64.b64encode(self.buffer_data).decode('ascii')
        
        # Build GLTF structure
        gltf = {
            "asset": {
                "version": "2.0",
                "generator": "CSV to GLTF Converter"
            },
            "scene": 0,
            "scenes": [
                {
                    "nodes": [0]
                }
            ],
            "nodes": [
                {
                    "mesh": 0
                }
            ],
            "meshes": [
                {
                    "primitives": [
                        {
                            "attributes": {
                                "POSITION": pos_acc,
                                "COLOR_0": col_acc,
                                "NORMAL": norm_acc,
                                "TEXCOORD_0": uv_acc
                            },
                            "indices": index_acc,
                            "mode": 4  # TRIANGLES
                        }
                    ]
                }
            ],
            "accessors": self.accessors,
            "bufferViews": self.buffer_views,
            "buffers": [
                {
                    "byteLength": len(self.buffer_data),
                    "uri": buffer_uri
                }
            ]
        }
        
        return gltf
        
    def save_gltf(self, output_file):
        """Save GLTF to file."""
        gltf = self.build_gltf()
        
        with open(output_file, 'w') as f:
            json.dump(gltf, f, indent=2)
            
        print(f"GLTF saved to {output_file}")
        print(f"Model contains {len(self.vertices)} vertices")


def main():
    if len(sys.argv) < 2:
        print("Usage: python csv_to_gltf.py <input.csv> [output.gltf]")
        print("Example: python csv_to_gltf.py mario.csv mario.gltf")
        sys.exit(1)
        
    input_file = sys.argv[1]
    
    # Default output filename
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    else:
        input_path = Path(input_file)
        output_file = input_path.with_suffix('.gltf')
        
    # Build GLTF
    builder = GLTFBuilder()
    builder.load_csv_vertices(input_file)
    builder.save_gltf(output_file)


if __name__ == "__main__":
    main()