#!/usr/bin/env python3
"""
Embed web files as PROGMEM strings in C++ header files
Converts HTML, CSS, JS files to compressed C++ constants
"""

import os
import gzip
import mimetypes
from pathlib import Path

def compress_data(data):
    """Compress data using gzip"""
    return gzip.compress(data.encode('utf-8') if isinstance(data, str) else data)

def bytes_to_cpp_array(data, var_name):
    """Convert bytes to C++ array declaration"""
    hex_data = ', '.join(f'0x{b:02x}' for b in data)
    return f"const uint8_t {var_name}[] PROGMEM = {{{hex_data}}};"

def get_mime_type(filename):
    """Get MIME type for file"""
    mime_type, _ = mimetypes.guess_type(filename)
    if mime_type is None:
        if filename.endswith('.js'):
            return 'application/javascript'
        elif filename.endswith('.css'):
            return 'text/css'
        elif filename.endswith('.html'):
            return 'text/html'
        elif filename.endswith('.svg'):
            return 'image/svg+xml'
        else:
            return 'application/octet-stream'
    return mime_type

def sanitize_var_name(filename):
    """Convert filename to valid C++ variable name"""
    # Remove path and extension, replace non-alphanumeric with underscore
    name = Path(filename).stem
    sanitized = ''.join(c if c.isalnum() else '_' for c in name)
    # Ensure it starts with letter or underscore
    if sanitized and sanitized[0].isdigit():
        sanitized = '_' + sanitized
    return sanitized or 'unnamed'

def process_web_files():
    """Process all web files and generate C++ header"""
    web_dir = Path('data/www')
    output_file = Path('src/web_files.h')
    
    if not web_dir.exists():
        print(f"Error: {web_dir} not found")
        return
    
    # File extensions to process
    extensions = {'.html', '.css', '.js', '.svg', '.ico', '.png', '.jpg', '.jpeg', '.gif', '.woff', '.woff2'}
    
    files_data = []
    
    # Process all files recursively
    for file_path in web_dir.rglob('*'):
        if file_path.is_file() and file_path.suffix.lower() in extensions:
            relative_path = file_path.relative_to(web_dir)
            url_path = '/' + str(relative_path).replace('\\', '/')
            
            print(f"Processing: {url_path}")
            
            # Read file
            if file_path.suffix.lower() in {'.png', '.jpg', '.jpeg', '.gif', '.ico', '.woff', '.woff2'}:
                # Binary files
                with open(file_path, 'rb') as f:
                    data = f.read()
                compressed = gzip.compress(data)
            else:
                # Text files
                with open(file_path, 'r', encoding='utf-8') as f:
                    data = f.read()
                compressed = compress_data(data)
            
            # Generate variable name
            var_name = f"web_{sanitize_var_name(str(relative_path).replace('/', '_').replace('.', '_'))}"
            
            # Store file info
            files_data.append({
                'url_path': url_path,
                'var_name': var_name,
                'data': compressed,
                'mime_type': get_mime_type(str(file_path)),
                'original_size': len(data) if isinstance(data, str) else len(data),
                'compressed_size': len(compressed)
            })
    
    # Generate header file
    header_content = '''#ifndef WEB_FILES_H
#define WEB_FILES_H

#include <Arduino.h>

struct WebFile {
    const char* path;
    const char* mime_type;
    const uint8_t* data;
    size_t size;
    bool gzipped;
};

'''
    
    # Add data arrays
    for file_info in files_data:
        header_content += bytes_to_cpp_array(file_info['data'], file_info['var_name']) + '\n'
    
    header_content += '\n// Web files array\n'
    header_content += f'const WebFile web_files[] = {{\n'
    
    for file_info in files_data:
        header_content += f'  {{"{file_info["url_path"]}", "{file_info["mime_type"]}", {file_info["var_name"]}, {file_info["compressed_size"]}, true}},\n'
    
    header_content += '};\n\n'
    header_content += f'const size_t web_files_count = {len(files_data)};\n\n'
    header_content += '#endif // WEB_FILES_H\n'
    
    # Write header file
    with open(output_file, 'w') as f:
        f.write(header_content)
    
    # Print summary
    total_original = sum(f['original_size'] for f in files_data)
    total_compressed = sum(f['compressed_size'] for f in files_data)
    compression_ratio = (1 - total_compressed / total_original) * 100 if total_original > 0 else 0
    
    print(f"\nGenerated {output_file}")
    print(f"Files processed: {len(files_data)}")
    print(f"Original size: {total_original:,} bytes")
    print(f"Compressed size: {total_compressed:,} bytes")
    print(f"Compression: {compression_ratio:.1f}%")

if __name__ == '__main__':
    process_web_files()