#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#

from __future__ import division, print_function

import argparse
import os
import sys

import file_splitter
from file_wrapper import FileHandleFactory, CombinedFileWriter
from json_reader import read_json


def load_descriptor(descriptor_filename):
    data = read_json(descriptor_filename)
    if not data["appname"] == "GIFT-Surg split data":
        raise ValueError('Not a GIFT-Surg file')
    if not data["version"] == "1.0":
        raise ValueError('Cannot read this file version')
    return data


def combine_file(input_file_base, descriptor_filename, filename_out_base, file_factory):
    """Combines several overlapping files into one output file"""

    if not filename_out_base:
        filename_out_base = os.path.splitext(filename_out_base)[0] + "_combined"
    filename_header = filename_out_base + '.mhd'

    if not descriptor_filename:
        [original_header, input_file_list] = generate_header_from_input_file_headers(input_file_base)
    else:
        [original_header, input_file_list] = generate_header_from_descriptor_file(descriptor_filename)

    input_combined = file_splitter.CombinedFileReader(input_file_base, input_file_list, file_factory)

    # Load in all descriptors for all files. We don't assume they are in order; we will use the index to order them
    # input_descriptors = sorted(input_file_list, key=lambda k: k['index'])

    output_image_size = original_header["DimSize"]
    descriptor_out = {}
    descriptor_out["index"] = 0
    descriptor_out["suffix"] = ""
    descriptor_out["filename"] = filename_header
    descriptor_out["ranges"] = [[0, output_image_size[0] - 1, 0, 0],
                                [0, output_image_size[1] - 1, 0, 0],
                                [0, output_image_size[2] - 1, 0, 0]]

    output_combined = CombinedFileWriter(filename_out_base, [descriptor_out], file_factory, original_header)
    output_combined.write_image_file(input_combined)

    input_combined.close()
    output_combined.close()


def generate_header_from_descriptor_file(descriptor_filename):
    descriptor = load_descriptor(descriptor_filename)
    original_file_list = descriptor["source_files"]
    if not len(original_file_list) == 1:
        raise ValueError('This function only supports data derived from a single file')
    original_file_descriptor = original_file_list[0]
    original_header = file_splitter.load_mhd_header(original_file_descriptor["filename"])
    input_file_list = descriptor["split_files"]
    return original_header, input_file_list


def generate_header_from_input_file_headers(input_file_base):

    file_index = 1
    suffix = str(file_index)
    file_name = input_file_base + suffix + '.mhd'

    if not os.path.isfile(file_name):
        raise ValueError('No file series found starting with ' + file_name)

    input_file_list = []

    current_ranges = None
    combined_header = None
    full_image_size = None
    while True:
        suffix = str(file_index)
        file_name = input_file_base + suffix + '.mhd'
        if not os.path.isfile(file_name):
            return combined_header, input_file_list
        current_header = file_splitter.load_mhd_header(file_name)
        current_image_size = current_header["DimSize"]
        if not current_ranges:
            full_image_size = current_image_size
            combined_header = current_header
            current_ranges = [[0, current_image_size[0] - 1, 0, 0],
                              [0, current_image_size[1] - 1, 0, 0],
                              [0, current_image_size[2] - 1, 0, 0]]
        else:
            if not current_image_size[0] == full_image_size[0]:
                raise ValueError('When loading without a descriptor file, the first dimension of each file must match')
            if not current_image_size[1] == full_image_size[1]:
                raise ValueError('When loading without a descriptor file, the second dimension of each file must match')
            full_image_size[2] = full_image_size[2] + current_image_size[2]
            current_ranges[2][0] = current_ranges[2][1] + 1
            current_ranges[2][1] = current_ranges[2][1] + current_image_size[2]

        # Update the combined image size
        combined_header["DimSize"] = full_image_size

        # Create a descriptor for this subimage
        descriptor = {}
        descriptor["index"] = file_index
        descriptor["suffix"] = suffix
        descriptor["filename"] = file_name
        descriptor["ranges"] = current_ranges
        input_file_list.append(descriptor)

        file_index += 1


def main(args):
    parser = argparse.ArgumentParser(description='Combines multiple image parts into a single large MetaIO (.mhd) file')

    parser.add_argument("-f", "--filename", required=True, default="_no_filename_specified",
                        help="Base name of files to combine")
    parser.add_argument("-o", "--out", required=False, default="", help="Filename of combined output file")
    parser.add_argument("-d", "--descriptor", required=False, default=None,
                        help="Name of descriptor file (.gift) which defines the file splitting")

    args = parser.parse_args(args)

    assert sys.version_info >= (3, 0)

    if args.filename == '_no_filename_specified':
        raise ValueError('No filename was specified')
    else:
        combine_file(args.filename, args.descriptor, args.out, FileHandleFactory())


if __name__ == '__main__':
    main(sys.argv[1:])
