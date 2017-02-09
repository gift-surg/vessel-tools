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
