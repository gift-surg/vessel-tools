# -*- coding: utf-8 -*-
from .context import tools
from tools import file_splitter
from tools.file_splitter import get_number_of_blocks

import unittest


class TestFileSplitter(unittest.TestCase):
    """Basic test cases."""

    def test_get_number_of_blocks(self):
        self.assertEqual(get_number_of_blocks([10, 10, 10], [5, 5, 5]), [2, 2, 2])
        self.assertEqual(get_number_of_blocks([9, 9, 9], [5, 5, 5]), [2, 2, 2])
        self.assertEqual(get_number_of_blocks([11, 11, 11], [5, 5, 5]), [3, 3, 3])
        self.assertEqual(get_number_of_blocks([2, 2, 2], [1, 2, 3]), [2, 1, 1])
        self.assertEqual(get_number_of_blocks([2001, 2000, 1999], [500, 500, 500]), [5, 4, 4])


if __name__ == '__main__':
    unittest.main()
