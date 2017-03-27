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
from collections import OrderedDict

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


def load_mhd_header(filename):
    """Return an OrderedDict containing metadata loaded from an mhd file"""

    metadata = OrderedDict()

    with open(filename) as header_file:
        for line in header_file:
            (key, val) = [x.strip() for x in line.split("=")]
            if key in ['ElementSpacing', 'Offset', 'CenterOfRotation', 'TransformMatrix']:
                val = [float(s) for s in val.split()]
            elif key in ['NDims', 'ElementNumberOfChannels']:
                val = int(val)
            elif key in ['DimSize']:
                val = [int(s) for s in val.split()]
            elif key in ['BinaryData', 'BinaryDataByteOrderMSB', 'CompressedData']:
                if val.lower() == "true":
                    val = True
                else:
                    val = False

            metadata[key] = val

    return metadata


def get_default_metadata():
    """Return an OrderedDict containing default mhd file metadata"""

    return OrderedDict(
        [('ObjectType', 'Image'), ('NDims', '3'), ('BinaryData', 'True'), ('BinaryDataByteOrderMSB', 'True'),
         ('CompressedData', []), ('CompressedDataSize', []), ('TransformMatrix', []), ('Offset', []),
         ('CenterOfRotation', []), ('AnatomicalOrientation', []), ('ElementSpacing', []), ('DimSize', []),
         ('ElementNumberOfChannels', []), ('ElementSize', []), ('ElementType', 'MET_FLOAT'), ('ElementDataFile', []),
         ('Comment', []), ('SeriesDescription', []), ('AcquisitionDate', []), ('AcquisitionTime', []),
         ('StudyDate', []), ('StudyTime', [])])


def save_mhd_header(filename, metadata):
    """Saves a mhd header file to disk using the given metadata"""

    # Add default metadata, replacing with custom specified values
    header = ''
    default_metadata = get_default_metadata()
    for key, val in default_metadata.items():
        if key in metadata.keys():
            value = metadata[key]
        else:
            value = val
        if value:
            value = str(value)
            value = value.replace("[", "").replace("]", "").replace(",", "")
            header += '%s = %s\n' % (key, value)

    # Add any custom metadata tags
    for key, val in metadata.items():
        if key not in default_metadata.keys():
            value = str(metadata[key])
            value = value.replace("[", "").replace("]", "").replace(",", "")
            header += '%s = %s\n' % (key, value)

    f = open(filename, 'w')
    f.write(header)
    f.close()


def get_bytes_per_voxel(element_type):
    """Returns number of bytes required to store one voxel for the given metaIO ElementType"""

    switcher = {
        'MET_CHAR': 1,
        'MET_UCHAR': 1,
        'MET_SHORT': 2,
        'MET_USHORT': 2,
        'MET_INT': 4,
        'MET_UINT': 4,
        'MET_FLOAT': 4,
        'MET_DOUBLE': 8,
    }
    return switcher.get(element_type, 2)


def create_file_from_range(output_filename, range_coords_in, file_in_streamer, metadata, bytes_per_voxel_out):
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
        write_file_range_to_file(file_in_streamer, file_out_streamer, range_coords_in, range_coords_out)


def write_file_range_to_file(file_in_streamer, file_out_streamer, range_coords_in, range_coords_out):
    i_range_in = range_coords_in[0]
    j_range_in = range_coords_in[1]
    k_range_in = range_coords_in[2]
    i_range_out = range_coords_out[0]
    j_range_out = range_coords_out[1]
    k_range_out = range_coords_out[2]

    for k_in, k_out in zip(range(k_range_in[0], 1 + k_range_in[1]), range(k_range_out[0], 1 + k_range_out[1])):
        for j_in, j_out in zip(range(j_range_in[0], 1 + j_range_in[1]), range(j_range_out[0], 1 + j_range_out[1])):
            start_coords_in = [i_range_in[0], j_in, k_in]
            num_voxels_to_read = i_range_in[1] + 1 - i_range_in[0]
            image_line = file_in_streamer.read_image_stream(start_coords_in, num_voxels_to_read)
            start_coords_out = [i_range_out[0], j_out, k_out]
            file_out_streamer.write_image_stream(start_coords_out, image_line)


def split_file(input_file, filename_out_base, max_block_size_voxels, overlap_size_voxels):
    """Saves the specified image file as a number of smaller files"""

    if not filename_out_base:
        filename_out_base = os.path.splitext(input_file)[0] + "_split"

    header = load_mhd_header(input_file)
    relative_filename_raw = header["ElementDataFile"]
    input_path = os.path.dirname(input_file)
    filename_raw = os.path.join(input_path, relative_filename_raw)
    file_reader = HugeFileHandle(filename_raw)
    image_size = header["DimSize"]
    num_dims = header["NDims"]
    bytes_per_voxel = get_bytes_per_voxel(header["ElementType"])
    max_block_size_voxels_array = convert_to_array(max_block_size_voxels, "block size", num_dims)
    overlap_voxels_size_array = convert_to_array(overlap_size_voxels, "overlap size", num_dims)

    ranges = get_image_block_ranges(image_size, max_block_size_voxels_array, overlap_voxels_size_array)

    descriptor = {"appname": "GIFT-Surg split data", "version": "1.0"}

    original_file_list = []
    original_file_descriptor = {"filename": input_file, "ranges": [[0, image_size[0] - 1], [0, image_size[1] - 1],
                                                                   [0, image_size[2] - 1]], "suffix": "", "index": 0}
    original_file_list.append(original_file_descriptor)

    split_file_list = []
    with file_reader as file_in:
        file_in_streamer = HugeFileStreamer(file_in, image_size, bytes_per_voxel)
        index = 0
        for subimage_range in ranges:
            suffix = "_" + str(index)
            output_filename = filename_out_base + suffix
            create_file_from_range(output_filename, subimage_range, file_in_streamer, header, bytes_per_voxel)
            file_descriptor = {"filename": output_filename, "ranges": subimage_range, "suffix": suffix, "index": index}
            split_file_list.append(file_descriptor)

            index += 1

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


class HugeFileStreamer:
    def __init__(self, file_handle_object, image_size, bytes_per_voxel):
        self._bytes_per_voxel = bytes_per_voxel
        self._image_size = image_size
        self._file_handle_object = file_handle_object

    def read_image_stream(self, start_coords, num_voxels_to_read):
        """Reads a line of image data from a binary file at the specified image location"""

        offset = self.get_linear_byte_offset(self._image_size, self._bytes_per_voxel, start_coords)
        self._file_handle_object.seek(offset)
        return self._file_handle_object.read(num_voxels_to_read*self._bytes_per_voxel)

    @staticmethod
    def get_linear_byte_offset(image_size, bytes_per_voxel, start_coords):
        """For a stream of bytes representing a multi-dimensional image, returns the byte offset corresponding to the
        point at the given coordinates """

        offset = 0
        offset_multiple = bytes_per_voxel
        for coord, image_length in zip(start_coords, image_size):
            offset += coord*offset_multiple
            offset_multiple *= image_length
        return offset


class HugeFileHandle:
    def __init__(self, name):
        self.filename = name

    def __enter__(self):
        self.file_handle = open(self.filename, 'rb')
        return self.file_handle

    def __exit__(self, type, value, traceback):
        self.file_handle.close()


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
        split_file(args.filename, args.out, args.max, args.overlap)


if __name__ == '__main__':
    main(sys.argv[1:])
