# -*- coding: utf-8 -*-
import math

from nose_parameterized import parameterized
from tools.file_splitter import get_number_of_blocks, get_block_coordinate_range, get_image_block_ranges, \
    get_suggested_block_size, get_bytes_per_voxel, HugeFileStreamer

import unittest


class FakeFile:
    def __init__(self, data, bytes_per_voxel):
        self.data = data
        self.bytes_per_voxel = bytes_per_voxel
        self.data_pointer = 0

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        pass

    def seek(self, num_bytes):
        self.data_pointer = math.floor(num_bytes/self.bytes_per_voxel)

    def read(self, num_bytes):
        return self.data[slice(self.data_pointer, self.data_pointer + math.floor(num_bytes/self.bytes_per_voxel))]


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

    def test_get_suggested_block_size(self):
        self.assertEqual(get_suggested_block_size([5, 5, 5], [2, 2, 2]), [3, 3, 3])
        self.assertEqual(get_suggested_block_size([5, 5, 5], [2, 3, 4]), [3, 2, 2])
        self.assertEqual(get_suggested_block_size([99, 100, 101], [3, 4, 5]), [33, 25, 21])

    def test_get_image_block_ranges(self):
        self.assertEqual(get_image_block_ranges([5, 5, 5], [4, 5, 6], [0, 0, 0]),
                         [[(0, 2), (0, 4), (0, 4)], [(3, 4), (0, 4), (0, 4)]])
        self.assertEqual(get_image_block_ranges([5, 5, 5], [4, 5, 6], [2, 2, 2]),
                         [[(0, 4), (0, 4), (0, 4)], [(1, 4), (0, 4), (0, 4)]])
        self.assertEqual(get_image_block_ranges([999, 1000, 1001], [500, 500, 500], [0, 0, 0]),
                         [[(0, 499), (0, 499), (0, 333)], [(0, 499), (0, 499), (334, 667)],
                          [(0, 499), (0, 499), (668, 1000)],
                          [(0, 499), (500, 999), (0, 333)], [(0, 499), (500, 999), (334, 667)],
                          [(0, 499), (500, 999), (668, 1000)],
                          [(500, 998), (0, 499), (0, 333)], [(500, 998), (0, 499), (334, 667)],
                          [(500, 998), (0, 499), (668, 1000)],
                          [(500, 998), (500, 999), (0, 333)], [(500, 998), (500, 999), (334, 667)],
                          [(500, 998), (500, 999), (668, 1000)]])

    def test_get_linear_byte_offset(self):
        self.assertEqual(HugeFileStreamer.get_linear_byte_offset([11, 22, 33], 4, [1, 2, 3]), (1+2*11+3*11*22)*4)
        self.assertEqual(HugeFileStreamer.get_linear_byte_offset([11, 22, 33, 44], 4, [1, 2, 3, 4]), (1+2*11+3*11*22+4*11*22*33)*4)
        self.assertEqual(HugeFileStreamer.get_linear_byte_offset([11, 22, 33], 1, [1, 2, 3]), (1+2*11+3*11*22)*1)
        self.assertEqual(HugeFileStreamer.get_linear_byte_offset([11, 22, 33], 4, [0, 2, 3]), (0+2*11+3*11*22)*4)
        self.assertEqual(HugeFileStreamer.get_linear_byte_offset([55, 301, 999], 7, [14, 208, 88]), (14+208*55+88*55*301)*7)

    def test_get_bytes_per_voxel(self):
        self.assertEquals(get_bytes_per_voxel('MET_CHAR'), 1)
        self.assertEquals(get_bytes_per_voxel('MET_UCHAR'), 1)
        self.assertEquals(get_bytes_per_voxel('MET_INT'), 4)
        self.assertEquals(get_bytes_per_voxel('MET_UINT'), 4)
        self.assertEquals(get_bytes_per_voxel('MET_SHORT'), 2)
        self.assertEquals(get_bytes_per_voxel('MET_USHORT'), 2)
        self.assertEquals(get_bytes_per_voxel('MET_FLOAT'), 4)
        self.assertEquals(get_bytes_per_voxel('MET_DOUBLE'), 8)

    @parameterized.expand([
        [[2, 3, 8], 4, [1, 2, 3], 2],
        [[101, 222, 4], 4, [1, 1, 1], 10],
        [[154, 141, 183], 4, [13, 12, 11], 30],
    ])
    def test_read_image_stream(self, image_size, bytes_per_voxel, start_coords, num_voxels_to_read):
        fake_file = FakeFile(range(0, image_size[0]*image_size[1]*image_size[2]-1), bytes_per_voxel)
        file_streamer = HugeFileStreamer(fake_file)
        start = start_coords[0] + start_coords[1]*image_size[0] + start_coords[2]*image_size[0]*image_size[1]
        end = start + num_voxels_to_read
        expected = range(start, end)
        self.assertEquals(file_streamer.read_image_stream(image_size, bytes_per_voxel, start_coords, num_voxels_to_read),
                          expected)

if __name__ == '__main__':
    unittest.main()

