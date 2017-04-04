#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#
import copy
import os
from collections import OrderedDict


class CombinedFileWriter:
    def __init__(self, output_file_base, descriptors, file_factory, header_template):
        descriptors_sorted = sorted(descriptors, key=lambda k: k['index'])
        self._subimages = []
        self._cached_last_subimage = None
        self._bytes_per_voxel = compute_bytes_per_voxel(header_template["ElementType"])
        for descriptor in descriptors_sorted:
            self._subimages.append(SubImage(output_file_base, descriptor, file_factory, header_template))

    def write_image_file(self, input_combined):
        for next_image in self._subimages:
            output_ranges = next_image.get_ranges()
            i_range_global = output_ranges[0]
            j_range_global = output_ranges[1]
            k_range_global = output_ranges[2]
            num_voxels_to_read_per_line = i_range_global[1] + 1 - i_range_global[0]

            for k_global in range(k_range_global[0], 1 + k_range_global[1]):
                for j_global in range(j_range_global[0], 1 + j_range_global[1]):
                    start_coords_global = [i_range_global[0], j_global, k_global]
                    image_line = input_combined.read_image_stream(start_coords_global, num_voxels_to_read_per_line)
                    next_image.write_image_stream(start_coords_global, image_line)

    def close(self):
        for subimage in self._subimages:
            subimage.close()


class CombinedFileReader:
    def __init__(self, input_file_base, descriptors, file_factory):
        descriptors_sorted = sorted(descriptors, key=lambda k: k['index'])
        self._subimages = []
        self._cached_last_subimage = None
        for descriptor in descriptors_sorted:
            self._subimages.append(SubImage(input_file_base, descriptor, file_factory, None))

    def read_image_stream(self, start_coords_global, num_voxels_to_read):
        byte_stream = b''
        current_i_start = start_coords_global[0]
        while num_voxels_to_read > 0:
            current_start_coords = [current_i_start, start_coords_global[1], start_coords_global[2]]
            next_image = self._find_subimage(current_start_coords, True)
            next_byte_stream = next_image.read_image_stream(current_start_coords, num_voxels_to_read)
            byte_stream += next_byte_stream
            bytes_per_voxel = next_image.get_bytes_per_voxel()
            num_voxels_read = round(len(next_byte_stream)/bytes_per_voxel)
            num_voxels_to_read -= num_voxels_read
            current_i_start += num_voxels_read
        return byte_stream

    def close(self):
        for subimage in self._subimages:
            subimage.close()

    def _find_subimage(self, start_coords_global, must_be_in_roi):

        # For efficiency, first check the last subimage before going through the whole list
        if self._cached_last_subimage \
                and self._cached_last_subimage.contains_voxel(start_coords_global, must_be_in_roi):
            return self._cached_last_subimage

        # Iterate through the list of subimages to find the one containing these start coordinates
        for next_subimage in self._subimages:
            if next_subimage.contains_voxel(start_coords_global, must_be_in_roi):
                self._cached_last_subimage = next_subimage
                return next_subimage

        raise ValueError('Coordinates are out of range')


class SubImage:
    def __init__(self, input_file_base, descriptor, file_factory, header_template):
        self._descriptor = descriptor

        # Construct the origin offset used to convert from global coordinates. This excludes overlapping voxels
        self._image_size = [1 + this_range[1] - this_range[0] for this_range in descriptor["ranges"]]
        self._origin_start = [this_range[0] for this_range in descriptor["ranges"]]
        self._origin_end = [this_range[1] for this_range in descriptor["ranges"]]
        self._roi_start = [this_range[0] + this_range[2] for this_range in descriptor["ranges"]]
        self._roi_end = [this_range[1] - this_range[3] for this_range in descriptor["ranges"]]
        self._ranges = descriptor["ranges"]

        if header_template:
            header_template["DimSize"] = self._image_size
            header_template["Origin"] = self._origin_start
        self._file = MetaIoFile(input_file_base + descriptor["suffix"] + ".mhd", file_factory, header_template)

    def get_ranges(self):
        """Returns the full range of global coordinates covered by this subimage"""

        return self._ranges

    def write_image_stream(self, start_coords, image_line):
        """Writes a line of image data to a binary file at the specified image location"""

        start_coords_local = self._convert_coords_to_local(start_coords)
        self._file.write_image_stream(start_coords_local, image_line)

    def read_image_stream(self, start_coords, num_voxels_to_read):
        """Reads a line of image data from a binary file at the specified image location"""

        if not self.contains_voxel(start_coords, True):
            raise ValueError('The data range to load extends beyond this file')

        # Don't read bytes beyond the end of the valid range
        if start_coords[0] + num_voxels_to_read - 1 > self._roi_end[0]:
            num_voxels_to_read = self._roi_end[0] - start_coords[0] + 1

        start_coords_local = self._convert_coords_to_local(start_coords)
        return self._file.read_image_stream(start_coords_local, num_voxels_to_read)

    def contains_voxel(self, start_coords_global, must_be_in_roi):
        """Determines if the specified voxel lies within the ROI of this subimage """

        if must_be_in_roi:
            return (self._roi_start[0] <= start_coords_global[0] <= self._roi_end[0] and
                    self._roi_start[1] <= start_coords_global[1] <= self._roi_end[1] and
                    self._roi_start[2] <= start_coords_global[2] <= self._roi_end[2])
        else:
            return (self._origin_start[0] <= start_coords_global[0] <= self._origin_end[0] and
                    self._origin_start[1] <= start_coords_global[1] <= self._origin_end[1] and
                    self._origin_start[2] <= start_coords_global[2] <= self._origin_end[2])

    def close(self):
        self._file.close()

    def get_bytes_per_voxel(self):
        return self._file.get_bytes_per_voxel()

    def _convert_coords_to_local(self, start_coords):
        return [start_coord - origin_coord for start_coord, origin_coord in zip(start_coords, self._origin_start)]


class MetaIoFile:
    def __init__(self, header_filename, file_factory, header_template):
        self._file_factory = file_factory
        self._header_filename = header_filename
        self._input_path = os.path.dirname(os.path.abspath(header_filename))
        self._file_wrapper = None
        self._file_streamer = None
        if header_template:
            # File is for writing
            self._mode = 'wb'
            # Force the raw filename to match the header filename
            base_filename = os.path.splitext(header_filename)[0]
            header = copy.deepcopy(header_template)
            header['ElementDataFile'] = base_filename + '.raw'

            save_mhd_header(header_filename, header)
            self._header = header

        else:
            # File is for reading
            self._mode = 'rb'
            self._header = None

    def write_image_stream(self, start_coords, image_line):
        """Writes a line of image data to a binary file at the specified image location"""

        return self._get_file_streamer().write_image_stream(start_coords, image_line)

    def read_image_stream(self, start_coords, num_voxels_to_read):
        """Reads a line of image data from a binary file at the specified image location"""

        return self._get_file_streamer().read_image_stream(start_coords, num_voxels_to_read)

    def get_bytes_per_voxel(self):
        header = self._get_header()
        return compute_bytes_per_voxel(header["ElementType"])

    def _get_header(self):
        if not self._header:
            self._header = load_mhd_header(self._header_filename)
        return self._header

    def _get_file_wrapper(self):
        if not self._file_wrapper:
            header = self._get_header()
            filename_raw = os.path.join(self._input_path, header["ElementDataFile"])
            self._file_wrapper = HugeFileWrapper(filename_raw, self._file_factory, self._mode)
        return self._file_wrapper

    def _get_file_streamer(self):
        if not self._file_streamer:
            header = self._get_header()
            bytes_per_voxel = compute_bytes_per_voxel(header["ElementType"])
            subimage_size = header["DimSize"]
            self._file_streamer = HugeFileStreamer(self._get_file_wrapper(), subimage_size, bytes_per_voxel)
        return self._file_streamer

    def close(self):
        if self._file_streamer:
            self._file_streamer.close()
            self._file_streamer = None
        if self._file_wrapper:
            self._file_wrapper.close()
            self._file_wrapper = None


class HugeFileStreamer:
    """A class to handle streaming of image data with arbitrarily large files"""

    def __init__(self, file_wrapper, image_size, bytes_per_voxel):
        self._bytes_per_voxel = bytes_per_voxel
        self._image_size = image_size
        self._file_wrapper = file_wrapper

    def read_image_stream(self, start_coords, num_voxels_to_read):
        """Reads a line of image data from a binary file at the specified image location"""

        offset = self._get_linear_byte_offset(self._image_size, self._bytes_per_voxel, start_coords)
        self._file_wrapper.get_handle().seek(offset)
        return self._file_wrapper.get_handle().read(num_voxels_to_read * self._bytes_per_voxel)

    def write_image_stream(self, start_coords, image_line):
        """Writes a line of image data to a binary file at the specified image location"""

        offset = self._get_linear_byte_offset(self._image_size, self._bytes_per_voxel, start_coords)
        self._file_wrapper.get_handle().seek(offset)
        bytes_written = self._file_wrapper.get_handle().write(image_line)
        if bytes_written != len(image_line):
            raise ValueError('Unexpected number of bytes written')

    @staticmethod
    def _get_linear_byte_offset(image_size, bytes_per_voxel, start_coords):
        """For a stream of bytes representing a multi-dimensional image, returns the byte offset corresponding to the
        point at the given coordinates """

        offset = 0
        offset_multiple = bytes_per_voxel
        for coord, image_length in zip(start_coords, image_size):
            offset += coord * offset_multiple
            offset_multiple *= image_length
        return offset

    def close(self):
        self._file_wrapper.close()


class HugeFileWrapper:
    """A class to handle arbitrarily large files"""

    def __init__(self, name, file_handle_factory, mode):
        self._file_handle_factory = file_handle_factory
        self._filename = name
        self._mode = mode
        self._file_handle = None

    def __del__(self):
        self.close()

    def __enter__(self):
        self.open()
        return self._file_handle

    def __exit__(self, exit_type, value, traceback):
        self.close()

    def get_handle(self):
        if not self._file_handle:
            self.open()
        return self._file_handle

    def open(self):
        self._file_handle = self._file_handle_factory.create_file_handle(self._filename, self._mode)

    def close(self):
        if self._file_handle and not self._file_handle.closed:
            self._file_handle.close()
            self._file_handle = None


class FileHandleFactory:
    @staticmethod
    def create_file_handle(filename, mode):
        return open(filename, mode)


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


def compute_bytes_per_voxel(element_type):
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


def get_default_metadata():
    """Return an OrderedDict containing default mhd file metadata"""

    return OrderedDict(
        [('ObjectType', 'Image'), ('NDims', '3'), ('BinaryData', 'True'), ('BinaryDataByteOrderMSB', 'True'),
         ('CompressedData', []), ('CompressedDataSize', []), ('TransformMatrix', []), ('Offset', []),
         ('CenterOfRotation', []), ('AnatomicalOrientation', []), ('ElementSpacing', []), ('DimSize', []),
         ('ElementNumberOfChannels', []), ('ElementSize', []), ('ElementType', 'MET_FLOAT'), ('ElementDataFile', []),
         ('Comment', []), ('SeriesDescription', []), ('AcquisitionDate', []), ('AcquisitionTime', []),
         ('StudyDate', []), ('StudyTime', [])])
