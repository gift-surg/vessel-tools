#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#

from __future__ import division, print_function

import argparse
import os
import sys
from math import ceil

from file_wrapper import CombinedFile, load_mhd_header, get_bytes_per_voxel, FileHandleFactory, save_mhd_header, \
    HugeFileStreamer
from json_reader import write_json


def get_number_of_blocks(image_size, max_block_size):
    """Returns a list containing the number of blocks in each dimension required to split the image into blocks that
    are subject to a maximum size limit"""

    return [int(ceil(image_size_element / max_block_size_element)) for image_size_element, max_block_size_element in
            zip(image_size, max_block_size)]


def get_block_coordinate_range(block_number, block_size, overlap_size, image_size):
    """Returns the minimum and maximum coordinate values in one dimension for an image block, where the dimension
    length image_size is to be split into the number of blocks specified by block_size with an overlap of overlap_size
    voxels at each boundary, and the current block_number is specified. There is no overlap at the outer border of the
    image, and the length of the final block is reduced if necessary so there is no padding"""

    # Compute the minimum coordinate of the block
    if block_number == 0:
        min_coord = 0
        start_border = 0
    else:
        min_coord = block_number * block_size - overlap_size
        start_border = overlap_size

    # Compute the maximum coordinate of the block
    end_border = overlap_size
    max_coord = int((block_number + 1) * block_size - 1 + overlap_size)
    if max_coord >= image_size:
        max_coord = image_size - 1
        end_border = 0

    return min_coord, max_coord, start_border, end_border


def get_suggested_block_size(image_size, number_of_blocks):
    """Returns a recommended block size (a list of the number of blocks in each dimension) to allow the specified
    image_size to be split into the specified number of blocks in each dimension, with each block being roughly
    equal in size"""

    return [ceil(image_size_element / number_of_blocks_element) for
            image_size_element, number_of_blocks_element
            in zip(image_size, number_of_blocks)]


def get_image_block_ranges(image_size, max_block_size, overlap_size):
    """Returns a list of ranges, where each recommended block size (a list of the number of blocks in each dimension)
    to allow the specified image_size to be split into the specified number of blocks in each dimension,
    with each block being roughly equal in size """

    number_of_blocks = get_number_of_blocks(image_size, max_block_size)
    suggested_block_size = get_suggested_block_size(image_size, number_of_blocks)
    block_ranges = []

    for i in range(number_of_blocks[0]):
        for j in range(number_of_blocks[1]):
            for k in range(number_of_blocks[2]):
                block_ranges.append(
                    [get_block_coordinate_range(index, block, overlap, size) for index, block, overlap, size in
                     zip([i, j, k], suggested_block_size, overlap_size, image_size)])

    return block_ranges


def create_file_from_range(output_filename, range_coords_in, input_combined, metadata, bytes_per_voxel_out):
    """Creates a subimage by reading the specified range of data from the file handle"""

    i_range = range_coords_in[0]
    j_range = range_coords_in[1]
    k_range = range_coords_in[2]
    image_segment_offset = [i_range[0], j_range[0], k_range[0]]
    image_segment_size = [1 + i_range[1] - i_range[0], 1 + j_range[1] - j_range[0], 1 + k_range[1] - k_range[0]]
    range_coords_out = [[0, image_segment_size[0]], [0, image_segment_size[1]], [0, image_segment_size[2]]]

    filename_header = output_filename + '.mhd'
    filename_raw = output_filename + '.raw'

    metadata_reduced = metadata
    metadata_reduced["DimSize"] = image_segment_size
    metadata_reduced["Origin"] = image_segment_offset
    metadata_reduced["ElementDataFile"] = filename_raw
    save_mhd_header(filename_header, metadata_reduced)

    with open(filename_raw, 'wb') as file_out:
        file_out_streamer = HugeFileOutStreamer(file_out, image_segment_size, bytes_per_voxel_out)
        write_file_range_to_file(input_combined, file_out_streamer, range_coords_out, range_coords_in)


def write_file_range_to_file(input_combined, file_out_streamer, range_coords_out, global_range):
    i_range_out = range_coords_out[0]
    j_range_out = range_coords_out[1]
    k_range_out = range_coords_out[2]
    i_range_global = global_range[0]
    j_range_global = global_range[1]
    k_range_global = global_range[2]

    for k_out, k_global in zip(range(k_range_out[0], 1 + k_range_out[1]),
                               range(k_range_global[0], 1 + k_range_global[1])):
        for j_out, j_global in zip(range(j_range_out[0], 1 + j_range_out[1]),
                                   range(j_range_global[0], 1 + j_range_global[1])):
            start_coords_global = [i_range_global[0], j_global, k_global]
            num_voxels_to_read = i_range_global[1] + 1 - i_range_global[0]
            image_line = input_combined.read_image_stream(start_coords_global, num_voxels_to_read)
            start_coords_out = [i_range_out[0], j_out, k_out]
            file_out_streamer.write_image_stream(start_coords_out, image_line)


def split_file(input_file, filename_out_base, max_block_size_voxels, overlap_size_voxels, file_factory):
    """Saves the specified image file as a number of smaller files"""

    if not filename_out_base:
        filename_out_base = os.path.splitext(input_file)[0] + "_split"

    header = load_mhd_header(input_file)
    image_size = header["DimSize"]
    num_dims = header["NDims"]
    bytes_per_voxel = get_bytes_per_voxel(header["ElementType"])
    max_block_size_voxels_array = convert_to_array(max_block_size_voxels, "block size", num_dims)
    overlap_voxels_size_array = convert_to_array(overlap_size_voxels, "overlap size", num_dims)

    ranges = get_image_block_ranges(image_size, max_block_size_voxels_array, overlap_voxels_size_array)

    descriptor = {"appname": "GIFT-Surg split data", "version": "1.0"}

    original_file_list = []
    original_file_descriptor = {"filename": input_file,
                                "ranges": [[0, image_size[0] - 1, 0, 0], [0, image_size[1] - 1, 0, 0],
                                           [0, image_size[2] - 1, 0, 0]], "suffix": "", "index": 0}
    original_file_list.append(original_file_descriptor)

    main_file_descriptor = FileDescriptor(input_file, 0, "", [0, image_size[0] - 1], [0, image_size[1] - 1],
                                          [0, image_size[2] - 1])

    split_file_list = []
    input_file_base = os.path.splitext(input_file)[0]
    input_combined = CombinedFile(input_file_base, original_file_list, file_factory)

    index = 0
    for subimage_range in ranges:
        suffix = "_" + str(index)
        output_filename = filename_out_base + suffix
        create_file_from_range(output_filename, subimage_range, input_combined, header, bytes_per_voxel)
        file_descriptor = {"filename": output_filename, "ranges": subimage_range, "suffix": suffix, "index": index}
        split_file_list.append(file_descriptor)

        index += 1

    input_combined.close()

    descriptor["split_files"] = split_file_list
    descriptor["source_files"] = original_file_list
    descriptor_output_filename = filename_out_base + "_info.gift"
    write_json(descriptor_output_filename, descriptor)


def convert_to_array(scalar_or_list, parameter_name, num_dims):
    if not isinstance(scalar_or_list, list):
        array = [scalar_or_list] * num_dims
    elif len(scalar_or_list) == num_dims:
        array = scalar_or_list
    else:
        raise ValueError('The ' + parameter_name + 'parameter must be a scalar, or a list containing one entry for '
                                                   'each image dimension')
    return array


class HugeFileOutStreamer:
    def __init__(self, file_handle_object, image_size, bytes_per_voxel):
        self._image_size = image_size
        self._bytes_per_voxel = bytes_per_voxel
        self._file_handle_object = file_handle_object

    def write_image_stream(self, start_coords, image_line):
        offset = HugeFileStreamer.get_linear_byte_offset(self._image_size, self._bytes_per_voxel, start_coords)
        self._file_handle_object.seek(offset)
        bytes_written = self._file_handle_object.write(image_line)
        if bytes_written != len(image_line):
            raise ValueError('Unexpected number of bytes written')


class FileDescriptor:
    def __init__(self, file_name, index, suffix, i_range, j_range, k_range):
        self.suffix = suffix
        self.index = index
        self.file_name = file_name
        self.i_range = i_range
        self.j_range = j_range
        self.k_range = k_range


def main(args):
    parser = argparse.ArgumentParser(description='Splits a large MetaIO (.mhd) file into multiple parts with overlap')

    parser.add_argument("-f", "--filename", required=True, default="_no_filename_specified",
                        help="Name of file to split")
    parser.add_argument("-o", "--out", required=False, default="", help="Prefix of output files")
    parser.add_argument("-l", "--overlap", required=False, default="50", type=int,
                        help="Number of voxels to overlap between outputs")
    parser.add_argument("-m", "--max", required=False, default="500", type=int,
                        help="Maximum number of voxels in each dimension")

    args = parser.parse_args(args)

    if args.filename == '_no_filename_specified':
        raise ValueError('No filename was specified')
    else:
        assert sys.version_info >= (3, 0)
        split_file(args.filename, args.out, args.max, args.overlap, FileHandleFactory())


if __name__ == '__main__':
    main(sys.argv[1:])
