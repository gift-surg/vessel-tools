#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#

from __future__ import division, print_function

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
    """Returns a list of ranges, where each recommended block size (a list of the number of blocks in each dimension) to allow the specified
    image_size to be split into the specified number of blocks in each dimension, with each block being roughly
    equal in size"""

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

    header = ''
    for key, val in get_default_metadata().items():
        if key in metadata.keys():
            value = metadata[key]
        else:
            value = val
        if value:
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

