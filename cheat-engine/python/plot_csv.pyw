import argparse
import csv
import sys
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

def load_vec3_csv(path):
    xs, ys, zs = [], [], []

    with open(path, newline="") as f:
        reader = csv.reader(f)
        rows = list(reader)

        # Detect header (non-numeric first row)
        start_row = 1 if rows and not is_float(rows[0][0]) else 0

        for row in rows[start_row:]:
            if len(row) < 3:
                continue
            try:
                x, y, z = map(float, row[:3])
                xs.append(x)
                ys.append(y)
                zs.append(z)
            except ValueError:
                continue
    
    # Hack: Add bounds points to prevent stretching.
    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    minz, maxz = min(zs), max(zs)
    avex = (minx + maxx) / 2
    avey = (miny + maxy) / 2
    avez = (minz + maxz) / 2

    range_max = max(maxx - minx, maxy - miny, maxz - minz)

    minx = avex - range_max/2
    maxx = avex + range_max/2
    miny = avey - range_max/2
    maxy = avey + range_max/2
    minz = avez - range_max/2
    maxz = avez + range_max/2

    xs.append(minx)
    xs.append(maxx)
    ys.append(miny)
    ys.append(maxy)
    zs.append(minz)
    zs.append(maxz)

    return xs, ys, zs

def load_csv_flat(path):
    values = []

    with open(path, newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            for item in row:
                try:
                    value = float(item)
                    values.append(value)
                except ValueError:
                    continue
    return values

def is_float(value):
    try:
        float(value)
        return True
    except ValueError:
        return False

def main():
    parser = argparse.ArgumentParser(description="Plot vec3 CSV as a 3D scatter plot")
    parser.add_argument("csv_path", help="Path to CSV file containing vec3 data")
    args = parser.parse_args()

    xs, ys, zs = load_vec3_csv(args.csv_path)

    if not xs:
        print("No valid vec3 data found in CSV.")
        sys.exit(1)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection="3d")

    ax.scatter(xs, ys, zs, s=10)

    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")

    ax.set_title("Vec3 Scatter Plot")

    plt.show()

if __name__ == "__main__":
    main()
