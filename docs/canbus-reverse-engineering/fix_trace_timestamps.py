#!/usr/bin/env python3
"""
Fix trace file timestamps to be sequential.

SavvyCAN may reorder frames based on timestamps. This script rewrites
timestamps to be in ascending order matching the line order, ensuring
playback happens in the correct sequence.

Usage:
    python fix_trace_timestamps.py input.csv [output.csv]
    
If output filename is not provided, it will append '_sequential' to the input name.
"""

import csv
import sys
import os


def fix_timestamps(input_file: str, output_file: str = None, interval_us: int = 1000):
    """
    Rewrite timestamps in a trace file to be sequential.
    
    Args:
        input_file: Path to input CSV trace file
        output_file: Path to output file (optional)
        interval_us: Microseconds between frames (default: 1000 = 1ms)
    """
    if output_file is None:
        base, ext = os.path.splitext(input_file)
        output_file = f"{base}_sequential{ext}"
    
    with open(input_file, 'r') as f_in, open(output_file, 'w', newline='') as f_out:
        reader = csv.reader(f_in)
        writer = csv.writer(f_out)
        
        # Copy header
        header = next(reader)
        writer.writerow(header)
        
        # Rewrite each row with sequential timestamp
        timestamp = 0
        line_count = 0
        for row in reader:
            if len(row) >= 1:
                row[0] = str(timestamp)
                writer.writerow(row)
                timestamp += interval_us
                line_count += 1
    
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print(f"Processed {line_count} frames")
    print(f"Timestamps: 0 to {timestamp - interval_us} us ({interval_us/1000:.1f}ms intervals)")
    
    return output_file


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    fix_timestamps(input_file, output_file)
