import os
import glob

src_dir = r"C:\Users\HP\.gemini\antigravity\scratch\pos-system\src"
count = 0
for ext in ['*.cpp', '*.h']:
    for f in glob.glob(os.path.join(src_dir, '**', ext), recursive=True):
        with open(f, 'rb') as fh:
            data = fh.read()
        # Check for UTF-8 BOM
        if data[:3] == b'\xef\xbb\xbf':
            with open(f, 'wb') as fh:
                fh.write(data[3:])
            print(f"Stripped BOM: {os.path.basename(f)}")
            count += 1
        # Check for UTF-16 LE BOM
        elif data[:2] == b'\xff\xfe':
            text = data.decode('utf-16-le')
            with open(f, 'wb') as fh:
                fh.write(text.encode('utf-8'))
            print(f"Converted UTF-16: {os.path.basename(f)}")
            count += 1
        else:
            print(f"OK: {os.path.basename(f)}")
print(f"\nFixed {count} files")
