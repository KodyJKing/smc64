#!/usr/bin/env python3
"""
Convert CSV data to a GLTF armature model.
Bone format: position, forward, right, up (no parenting data)
"""

import csv
import json
import struct
import base64
import sys
import math
from pathlib import Path


class GLTFArmatureBuilder:
    def __init__(self):
        self.bones = []
        self.buffer_data = b''
        self.accessors = []
        self.buffer_views = []
        self.nodes = []
        
    def load_csv_bones(self, csv_file):
        """Load bone data from CSV file."""
        print(f"Loading bone data from {csv_file}...")
        
        with open(csv_file, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line:
                    continue
                    
                # Split by comma and strip whitespace, removing empty strings
                values = [v.strip() for v in line.split(',') if v.strip()]
                
                if len(values) < 12:
                    print(f"Warning: Line {line_num} has {len(values)} values, need 12 for position(3) + forward(3) + right(3) + up(3). Skipping.")
                    continue
                    
                try:
                    # Parse bone data - position + 3 direction vectors
                    bone = {
                        'position': [float(values[0]), float(values[1]), float(values[2])],
                        'forward': [float(values[3]), float(values[4]), float(values[5])],
                        'right': [float(values[6]), float(values[7]), float(values[8])],
                        'up': [float(values[9]), float(values[10]), float(values[11])]
                    }
                    self.bones.append(bone)
                except (ValueError, IndexError) as e:
                    print(f"Warning: Line {line_num} has invalid values: {e}. Skipping.")
                    continue
        
        print(f"Loaded {len(self.bones)} bones")
        
    def normalize_vector(self, vec):
        """Normalize a 3D vector."""
        length = math.sqrt(vec[0]**2 + vec[1]**2 + vec[2]**2)
        if length == 0:
            return [0, 0, 1]  # Default up vector
        return [vec[0]/length, vec[1]/length, vec[2]/length]
        
    def vectors_to_quaternion(self, forward, right, up):
        """Convert three orthogonal vectors to a quaternion."""
        # Normalize vectors
        forward = self.normalize_vector(forward)
        right = self.normalize_vector(right)
        up = self.normalize_vector(up)
        
        # Create rotation matrix from basis vectors
        # Note: GLTF uses column-major matrices
        m00, m01, m02 = right
        m10, m11, m12 = up  
        m20, m21, m22 = forward #[-forward[0], -forward[1], -forward[2]]  # Negate forward for standard coordinate system
        
        # Convert rotation matrix to quaternion
        trace = m00 + m11 + m22
        
        if trace > 0:
            s = math.sqrt(trace + 1.0) * 2  # s = 4 * qw
            w = 0.25 * s
            x = (m21 - m12) / s
            y = (m02 - m20) / s
            z = (m10 - m01) / s
        elif m00 > m11 and m00 > m22:
            s = math.sqrt(1.0 + m00 - m11 - m22) * 2  # s = 4 * qx
            w = (m21 - m12) / s
            x = 0.25 * s
            y = (m01 + m10) / s
            z = (m02 + m20) / s
        elif m11 > m22:
            s = math.sqrt(1.0 + m11 - m00 - m22) * 2  # s = 4 * qy
            w = (m02 - m20) / s
            x = (m01 + m10) / s
            y = 0.25 * s
            z = (m12 + m21) / s
        else:
            s = math.sqrt(1.0 + m22 - m00 - m11) * 2  # s = 4 * qz
            w = (m10 - m01) / s
            x = (m02 + m20) / s
            y = (m12 + m21) / s
            z = 0.25 * s
            
        return [x, y, z, w]
        
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
        
    def create_joint_nodes(self):
        """Create joint nodes from bone data."""
        if not self.bones:
            raise ValueError("No bones loaded")
            
        for i, bone in enumerate(self.bones):
            # Convert basis vectors to quaternion
            quat = self.vectors_to_quaternion(bone['forward'], bone['right'], bone['up'])

            # Normalize quaternion
            length = math.sqrt(quat[0]**2 + quat[1]**2 + quat[2]**2 + quat[3]**2)
            if length > 0:
                quat = [q / length for q in quat]

            # Create joint node
            node = {
                "name": f"Bone_{i:03d}",
                "translation": bone['position'],
                "rotation": quat,
                "scale": [1.0, 1.0, 1.0]
            }
            
            self.nodes.append(node)
            
        return list(range(len(self.nodes)))
        
    def pack_joint_data(self):
        """Pack joint matrices for skinning."""
        if not self.bones:
            raise ValueError("No bones loaded")
            
        # Create inverse bind matrices (identity for now since we don't have bind pose data)
        matrices_data = b''
        identity_matrix = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]
        
        for _ in self.bones:
            matrices_data += struct.pack('<16f', *identity_matrix)
            
        matrix_offset = len(self.buffer_data)
        self.buffer_data += matrices_data
        matrix_view = self.create_buffer_view(matrix_offset, len(matrices_data))
        matrix_accessor = self.create_accessor(matrix_view, 5126, len(self.bones), "MAT4")  # FLOAT
        
        return matrix_accessor
        
    def create_simple_mesh(self):
        """Create a simple visualization mesh for the armature."""
        # Create simple line segments connecting bones
        vertices = []
        indices = []
        
        for i, bone in enumerate(self.bones):
            pos = bone['position']
            forward = bone['forward']
            right = bone['right']
            up = bone['up']
            
            # Add bone origin
            vertices.extend(pos)
            
            # Add bone direction indicators (short lines)
            scale = 10.0  # Adjust based on your coordinate system
            vertices.extend([pos[0] + forward[0] * scale, pos[1] + forward[1] * scale, pos[2] + forward[2] * scale])
            vertices.extend([pos[0] + right[0] * scale, pos[1] + right[1] * scale, pos[2] + right[2] * scale])
            vertices.extend([pos[0] + up[0] * scale, pos[1] + up[1] * scale, pos[2] + up[2] * scale])
            
            base_idx = i * 4
            # Lines from origin to direction vectors
            indices.extend([base_idx, base_idx + 1])  # forward
            indices.extend([base_idx, base_idx + 2])  # right  
            indices.extend([base_idx, base_idx + 3])  # up
            
        # Pack vertex data
        vertex_data = struct.pack(f'<{len(vertices)}f', *vertices)
        vertex_offset = len(self.buffer_data)
        self.buffer_data += vertex_data
        vertex_view = self.create_buffer_view(vertex_offset, len(vertex_data), 34962)  # ARRAY_BUFFER
        
        # Calculate bounds
        x_coords = [vertices[i] for i in range(0, len(vertices), 3)]
        y_coords = [vertices[i] for i in range(1, len(vertices), 3)]
        z_coords = [vertices[i] for i in range(2, len(vertices), 3)]
        min_pos = [min(x_coords), min(y_coords), min(z_coords)]
        max_pos = [max(x_coords), max(y_coords), max(z_coords)]
        
        vertex_accessor = self.create_accessor(vertex_view, 5126, len(vertices)//3, "VEC3", min_pos, max_pos)
        
        # Pack indices
        index_data = struct.pack(f'<{len(indices)}H', *indices)
        index_offset = len(self.buffer_data)
        self.buffer_data += index_data
        index_view = self.create_buffer_view(index_offset, len(index_data), 34963)  # ELEMENT_ARRAY_BUFFER
        index_accessor = self.create_accessor(index_view, 5123, len(indices), "SCALAR")  # UNSIGNED_SHORT
        
        return vertex_accessor, index_accessor
        
    def build_gltf(self):
        """Build the complete GLTF structure."""
        if not self.bones:
            raise ValueError("No bones loaded")
            
        # Create joint nodes
        joint_indices = self.create_joint_nodes()
        
        # Pack joint data
        inverse_bind_matrices = self.pack_joint_data()
        
        # Create visualization mesh
        vertex_accessor, index_accessor = self.create_simple_mesh()
        
        # Encode buffer as base64
        buffer_uri = "data:application/octet-stream;base64," + base64.b64encode(self.buffer_data).decode('ascii')
        
        # Build GLTF structure
        gltf = {
            "asset": {
                "version": "2.0",
                "generator": "CSV to Armature Converter"
            },
            "scene": 0,
            "scenes": [
                {
                    "nodes": [len(self.nodes)]  # Root node
                }
            ],
            "nodes": self.nodes + [
                {
                    "name": "Armature_Root",
                    "children": joint_indices,
                    "mesh": 0
                }
            ],
            "meshes": [
                {
                    "name": "Armature_Visualization",
                    "primitives": [
                        {
                            "attributes": {
                                "POSITION": vertex_accessor
                            },
                            "indices": index_accessor,
                            "mode": 1  # LINES
                        }
                    ]
                }
            ],
            "skins": [
                {
                    "name": "Armature_Skin",
                    "joints": joint_indices,
                    "inverseBindMatrices": inverse_bind_matrices
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
            
        print(f"GLTF armature saved to {output_file}")
        print(f"Armature contains {len(self.bones)} bones")


def main():
    if len(sys.argv) < 2:
        print("Usage: python csv_to_armature.py <input.csv> [output.gltf]")
        print("Example: python csv_to_armature.py bones.csv armature.gltf")
        print()
        print("Expected CSV format: position(3), forward(3), right(3), up(3)")
        print("Each line: px,py,pz, fx,fy,fz, rx,ry,rz, ux,uy,uz")
        sys.exit(1)
        
    input_file = sys.argv[1]
    
    # Default output filename
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    else:
        input_path = Path(input_file)
        output_file = input_path.with_suffix('.gltf')
        
    # Build GLTF armature
    builder = GLTFArmatureBuilder()
    builder.load_csv_bones(input_file)
    builder.save_gltf(output_file)


if __name__ == "__main__":
    main()

