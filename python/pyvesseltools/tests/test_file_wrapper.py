# -*- coding: utf-8 -*-
import math

from nose_parameterized import parameterized

import file_wrapper
from file_wrapper import HugeFileStreamer
from tools.file_splitter import get_bytes_per_voxel

import unittest


class FakeFileHandleFactory:
    def __init__(self, fake_file):
        self._fake_file = fake_file

    def create_file_handle(self, filename, mode):
        return self._fake_file


class FakeFile:
    def __init__(self, data, bytes_per_voxel):
        self.data = data
        self.bytes_per_voxel = bytes_per_voxel
        self.data_pointer = 0
        self.closed = True

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        pass

    def seek(self, num_bytes):
        self.data_pointer = math.floor(num_bytes/self.bytes_per_voxel)

    def read(self, num_bytes):
        return self.data[slice(self.data_pointer, self.data_pointer + math.floor(num_bytes/self.bytes_per_voxel))]

    def open(self):
        self.closed = False

    def close(self):
        self.closed = True


class TestFileWrapper(unittest.TestCase):
    """Tests for FileWrapper"""

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
        fake_file_factory = FakeFileHandleFactory(FakeFile(range(0, image_size[0]*image_size[1]*image_size[2]-1), bytes_per_voxel))
        wrapper = file_wrapper.HugeFileWrapper("abcde", fake_file_factory)
        file_streamer = HugeFileStreamer(wrapper, image_size, bytes_per_voxel)
        start = start_coords[0] + start_coords[1]*image_size[0] + start_coords[2]*image_size[0]*image_size[1]
        end = start + num_voxels_to_read
        expected = range(start, end)
        self.assertEquals(file_streamer.read_image_stream(start_coords, num_voxels_to_read), expected)

if __name__ == '__main__':
    unittest.main()

