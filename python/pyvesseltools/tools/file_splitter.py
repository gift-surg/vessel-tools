#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#

from __future__ import division, print_function

from math import ceil


def get_number_of_blocks(image_size, max_block_size):
    return [ceil(image_size_element / max_block_size_element) for image_size_element, max_block_size_element in
            zip(image_size, max_block_size)]


def get_block_coordinate_range(block_number, block_size, overlap_size, image_size):
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
    return [ceil(image_size_element / number_of_blocks_element) for
            image_size_element, number_of_blocks_element
            in zip(image_size, number_of_blocks)]


def get_image_block_ranges(image_size, max_block_size, overlap_size):
    number_of_blocks = get_number_of_blocks(image_size, max_block_size)
    suggested_block_size = get_suggested_block_size(image_size, number_of_blocks)
    block_ranges = []

    for i in range(number_of_blocks[0]):
        for j in range(number_of_blocks[1]):
            for k in range(number_of_blocks[2]):
                indices = [i, j, k]
                block_ranges.append(
                    [get_block_coordinate_range(index, block, overlap, size) for index, block, overlap, size in
                     zip(indices, suggested_block_size, overlap_size, image_size)])

    return block_ranges


