# -*- coding: utf-8 -*-

from context import tools
from tools import file_splitter
from tools.file_splitter import get_number_of_blocks, get_block_coordinate_range

import unittest


class TestFileSplitter(unittest.TestCase):
    """Tests for FileSplitter."""

    def test_get_number_of_blocks(self):
        self.assertEqual(get_number_of_blocks([10, 10, 10], [5, 5, 5]), [2, 2, 2])
        self.assertEqual(get_number_of_blocks([9, 9, 9], [5, 5, 5]), [2, 2, 2])
        self.assertEqual(get_number_of_blocks([11, 11, 11], [5, 5, 5]), [3, 3, 3])
        self.assertEqual(get_number_of_blocks([2, 2, 2], [1, 2, 3]), [2, 1, 1])
        self.assertEqual(get_number_of_blocks([2001, 2000, 1999], [500, 500, 500]), [5, 4, 4])

    def test_get_block_coordinate_range(self):
        self.assertEqual(get_block_coordinate_range(0, 5, 1, 5), (0, 4))
        self.assertEqual(get_block_coordinate_range(0, 5, 1, 10), (0, 5))
        self.assertEqual(get_block_coordinate_range(1, 5, 1, 10), (4, 9))
        self.assertEqual(get_block_coordinate_range(0, 5, 1, 9), (0, 5))
        self.assertEqual(get_block_coordinate_range(1, 5, 1, 9), (4, 8))


if __name__ == '__main__':
    unittest.main()
