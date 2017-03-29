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
from file_splitter import write_file_range_to_file, get_bytes_per_voxel, HugeFileHandle, HugeFileStreamer, \
    HugeFileOutStreamer
from json_reader import read_json


def load_descriptor(descriptor_filename):
    data = read_json(descriptor_filename)
    if not data["appname"] == "GIFT-Surg split data":
        raise ValueError('Not a GIFT-Surg file')
    if not data["version"] == "1.0":
        raise ValueError('Cannot read this file version')
    return data


def combine_file(input_file_base, descriptor_filename, filename_out):
    """Combines several overlapping files into one output file"""

    if not filename_out:
        filename_out = os.path.splitext(filename_out)[0] + "_combined"
    filename_header = filename_out + '.mhd'
    filename_raw = filename_out + '.raw'

    if not descriptor_filename:
        [original_header, input_file_list] = generate_header_from_input_file_headers(input_file_base)
    else:
        [original_header, input_file_list] = generate_header_from_descriptor_file(descriptor_filename)

    original_image_size = original_header["DimSize"]
    bytes_per_voxel_out = get_bytes_per_voxel(original_header["ElementType"])
    original_header['ElementDataFile'] = filename_raw
    file_splitter.save_mhd_header(filename_header, original_header)

    num_input_files = len(input_file_list)
    descriptors = [None] * num_input_files

    # Load in all descriptors for all files. We don't assume they are in order; we will use the index to order them
    for file_descriptor in input_file_list:
        index = file_descriptor["index"]
        descriptors[index] = file_descriptor

    with open(filename_raw, 'wb') as file_out:
        file_out_streamer = HugeFileOutStreamer(file_out, original_image_size, bytes_per_voxel_out)
        num_descriptors = len(descriptors)
        for file_index in range(0, num_descriptors):

            current_descriptor = descriptors[file_index]
            input_filename = input_file_base + current_descriptor["suffix"] + ".mhd"
            current_range = current_descriptor["ranges"]
            i_range = current_range[0]
            j_range = current_range[1]
            k_range = current_range[2]

            current_input_header = file_splitter.load_mhd_header(input_filename)
            filename_raw_in = current_input_header["ElementDataFile"]
            bytes_per_voxel = get_bytes_per_voxel(current_input_header["ElementType"])
            file_reader_in = HugeFileHandle(filename_raw_in)
            image_size_in = current_input_header["DimSize"]

            input_range = [[i_range[2], i_range[1] - i_range[0] - i_range[3]],
                           [j_range[2], j_range[1] - j_range[0] - j_range[3]],
                           [k_range[2], k_range[1] - k_range[0] - k_range[3]]]

            output_range = [[i_range[0] + i_range[2], i_range[1] - i_range[3]],
                            [j_range[0] + j_range[2], j_range[1] - j_range[3]],
                            [k_range[0] + k_range[2], k_range[1] - k_range[3]]]

            with file_reader_in as file_handle_in:
                file_in_streamer = HugeFileStreamer(file_handle_in, image_size_in, bytes_per_voxel)
                write_file_range_to_file(file_in_streamer, file_out_streamer, input_range, output_range)


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
            if not current_image_size[0] == full_image_size[0] - 1:
                raise ValueError('When loading without a descriptor file, the first dimension of each file must match')
            if not current_image_size[1] == full_image_size[1] - 1:
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
        combine_file(args.filename, None, args.out)


if __name__ == '__main__':
    main(sys.argv[1:])
