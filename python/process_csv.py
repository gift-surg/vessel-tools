"""
process_csv.py 
Reformats the results from weka segmentation and puts them into a single file.
Author: M.A. Zuluaga
"""
import xlwt
import csv
import glob
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--dir", required=True, help="CSV directory")
parser.add_argument("-o", "--out", required=True, help="Output file")
parser.add_argument("-e", "--ext", required=True, help="Extension of files")

args = parser.parse_args()
a_dirname = args.dir
out_file = args.out

# a_dirname="/home/mzuluaga/Dropbox/Histology
# out_file="/home/mzuluaga/Dropbox/Histology/file2.xls"


assert isinstance(args.ext, object)
pattern = a_dirname + "/*" + args.ext
print pattern

all_files = glob.glob(pattern)
num_counters = len(all_files)
counters = [0] * num_counters
max_rows = [0] * num_counters
f_count = 0
for filename in all_files:
    (f_path, f_name) = os.path.split(filename)
    spamReader = csv.reader(open(filename, 'rb'))
    for rowx, row in enumerate(spamReader):
        max_rows[f_count] += 1
    f_count += 1
if len(max_rows) > 0:
    wb = xlwt.Workbook()
    ws = wb.add_sheet("all_values")
    max_val_rows = max(max_rows)
    for r in range(0, max_val_rows, 1):
        for f in range(0, num_counters, 1):
            filename = all_files[f]
            spamReader = csv.reader(open(filename, 'rb'))
            for rowx, row in enumerate(spamReader):
                if rowx == counters[f] and counters[f] < max_rows[f]:
                    for colx, value in enumerate(row):
                        if colx == 1:
                            if rowx == 0:
                                (f_path, f_name) = os.path.split(filename)
                                (f_short_name, f_extension) = os.path.splitext(f_name)
                                ws.write(rowx, f, f_short_name)
                            else:
                                divided = value.split(',')
                                ws.write(rowx, f, value)
                    counters[f] += 1
                    break

    wb.save(out_file)
    print "Done!"
else:
    print "No files found. There is nothing to do"
