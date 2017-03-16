#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#

from __future__ import division, print_function

import os
from math import ceil
from collections import OrderedDict


def get_number_of_blocks(image_size, max_block_size):
    """Returns a list containing the number of blocks in each dimension required to split the image into blocks that
    are subject to a maximum size limit"""

    return [ceil(image_size_element / max_block_size_element) for image_size_element, max_block_size_element in
            zip(image_size, max_block_size)]


def get_block_coordinate_range(block_number, block_size, overlap_size, image_size):
    """Returns the minimum and maximum coordinate values in one dimension for an image block, where the dimension
    length image_size is to be split into the number of blocks specified by block_size with an overlap of overlap_size
    voxels at each boundary, and the current block_number is specified. There is no overlap at the outer border of the
    image, and the length of the final block is reduced if necessary so there is no padding"""

    # Compute the minimum coordinate of the block
    if block_number == 0:
        min_coord = 0
    else:
        min_coord = block_number * block_size - overlap_size

    # Compute the maximum coordinate of the block
    max_coord = (block_number + 1) * block_size - 1 + overlap_size
    if max_coord >= image_size:
        max_coord = image_size - 1

    return min_coord, max_coord


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


def get_linear_byte_offset(image_size, bytes_per_voxel, start_coords):
    """For a stream of bytes representing a multi-dimensional image, returns the byte offset corresponding to the
    point at the given coordinates """

    offset = 0
    offset_multiple = bytes_per_voxel
    for coord, image_length in zip(start_coords, image_size):
        offset += coord*offset_multiple
        offset_multiple *= image_length
    return offset


def read_image_stream(file_handle, image_size, bytes_per_voxel, start_coords, num_voxels_to_read):
    """Reads a line of image data from a binary file at the specified image location"""

    offset = get_linear_byte_offset(image_size, bytes_per_voxel, start_coords)
    file_handle.seek(offset)
    return file_handle.read(num_voxels_to_read*bytes_per_voxel)


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


def create_file_from_range(output_path, filename, range_coords, file_handle_in, metadata):
    """Creates a subimage by reading the specified range of data from the file handle"""

    i_range = range_coords[0]
    j_range = range_coords[1]
    k_range = range_coords[2]
    image_segment_offset = [i_range[0], j_range[0], k_range[0]]
    image_segment_size = [1 + i_range[1] - i_range[0], 1 + j_range[1] - j_range[0], 1 + k_range[1] - k_range[0]]

    image_size = metadata["DimSize"]
    bytes_per_voxel = get_bytes_per_voxel(metadata["ElementType"])
    filename_header = filename + '.mhd'
    full_filename_header = os.path.join(output_path, filename_header)
    filename_raw = filename + '.raw'
    full_filename_raw = os.path.join(output_path, filename_raw)

    metadata_reduced = metadata
    metadata_reduced["DimSize"] = image_segment_size
    metadata_reduced["Origin"] = image_segment_offset
    metadata_reduced["ElementDataFile"] = filename_raw
    save_mhd_header(full_filename_header, metadata_reduced)

    with open(full_filename_raw, 'wb') as file_out:
        for k in range(k_range[0], 1 + k_range[1]):
            for j in range(j_range[0], 1 + j_range[1]):
                i = i_range[0]
                start_coords = [i, j, k]
                num_voxels_to_read = image_segment_size[0]

                image_line = read_image_stream(file_handle_in, image_size, bytes_per_voxel, start_coords,
                                               num_voxels_to_read)
                bytes_written = file_out.write(image_line)
                if bytes_written != len(image_line):
                    raise ValueError('Unexpected number of bytes written')


def split_file(filename):
    """Saves the specified image file as a number of smaller files"""

    pathname = os.path.dirname(filename)
    header = load_mhd_header(filename)
    local_filename_raw = header["ElementDataFile"]
    filename_raw = os.path.join(pathname, local_filename_raw)
    read_and_split_file(header, HugeFileReader(filename_raw), pathname)


def read_and_split_file(header, file_reader, output_path):
    """Saves the specified image file handle as a number of smaller files"""

    max_block_size = [500, 500, 500]
    overlap_size = [10, 10, 10]
    image_size = header["DimSize"]
    ranges = get_image_block_ranges(image_size, max_block_size, overlap_size)
    filename_out_base = os.path.splitext(header["ElementDataFile"])[0]

    with file_reader as file_in:
        index = 0
        for subimage_range in ranges:
            filename_subimage_out = filename_out_base + "_" + str(index)
            create_file_from_range(output_path, filename_subimage_out, subimage_range, file_in, header)
            index += 1


class HugeFileReader:
    def __init__(self, name):
        self.filename = name

    def __enter__(self):
        self.file_handle = open(self.filename, 'rb')
        return self.file_handle

    def __exit__(self, type, value, traceback):
        self.file_handle.close()
