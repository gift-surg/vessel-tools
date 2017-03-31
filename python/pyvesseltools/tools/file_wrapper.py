#!/usr/bin/env python

#
# Copyright UCL 2017
# Author: Tom Doel
#
import os
import sys
from collections import OrderedDict


class CombinedFile:
    def __init__(self, input_file_base, descriptors, file_factory):
        descriptors_sorted = sorted(descriptors, key=lambda k: k['index'])
        self._subimages = []
        self._cached_last_subimage = None
        for descriptor in descriptors_sorted:
            self._subimages.append(SubImage(input_file_base, descriptor, file_factory))

    def read_image_stream(self, start_coords_global, num_voxels_to_read):
        return self._find_subimage(start_coords_global).read_image_stream(start_coords_global, num_voxels_to_read)

    def close(self):
        for subimage in self._subimages:
            subimage.close()

    def _find_subimage(self, start_coords_global):

        # For efficiency, first check the last subimage before going through the whole list
        if self._cached_last_subimage and self._cached_last_subimage.contains_voxel(start_coords_global):
            return self._cached_last_subimage

        # Iterate through the list of subimages to find the one containing these start coordinates
        for next_subimage in self._subimages:
            if next_subimage.contains_voxel(start_coords_global):
                self._cached_last_subimage = next_subimage
                return next_subimage

        raise ValueError('Coordinates are out of range')


class SubImage:
    def __init__(self, input_file_base, descriptor, file_factory):
        self._descriptor = descriptor
        self._file = MetaIoFile(input_file_base + descriptor["suffix"] + ".mhd", file_factory)

        # Construct the origin offset used to convert from global coordinates. This excludes overlapping voxels
        self._origin_start = [this_range[0] for this_range in descriptor["ranges"]]
        self._roi_start = [this_range[0] + this_range[2] for this_range in descriptor["ranges"]]
        self._roi_end = [this_range[1] - this_range[3] for this_range in descriptor["ranges"]]

    def read_image_stream(self, start_coords, num_voxels_to_read):
        """Reads a line of image data from a binary file at the specified image location"""

        end_coords = [start_coords[0] + num_voxels_to_read - 1, start_coords[1], start_coords[2]]
        if not self.contains_voxel(start_coords) or not self.contains_voxel(end_coords):
            raise ValueError('The data range to load extends beyond this file')

        start_coords_local = self._convert_coords_to_local(start_coords)
        return self._file.read_image_stream(start_coords_local, num_voxels_to_read)

    def contains_voxel(self, start_coords_global):
        """Determines if the specified voxel lies within the ROI of this subimage """

        return (self._roi_start[0] <= start_coords_global[0] <= self._roi_end[0] and
                self._roi_start[1] <= start_coords_global[1] <= self._roi_end[1] and
                self._roi_start[2] <= start_coords_global[2] <= self._roi_end[2])

    def close(self):
        self._file.close()

    def _convert_coords_to_local(self, start_coords):
        return [start_coord - origin_coord for start_coord, origin_coord in zip(start_coords, self._origin_start)]


class MetaIoFile:
    def __init__(self, header_filename, file_factory):
        self._file_factory = file_factory
        self._header_filename = header_filename
        self._input_path = os.path.dirname(os.path.abspath(header_filename))
        self._header = None
        self._file_wrapper = None
        self._file_streamer = None

    def read_image_stream(self, start_coords, num_voxels_to_read):
        """Reads a line of image data from a binary file at the specified image location"""

        file_in_streamer = self._get_file_streamer()
        # file_in_streamer.open
        return file_in_streamer.read_image_stream(start_coords, num_voxels_to_read)

    def _get_header(self):
        if not self._header:
            self._header = load_mhd_header(self._header_filename)
        return self._header

    def _get_file_wrapper(self):
        if not self._file_wrapper:
            header = self._get_header()
            filename_raw = os.path.join(self._input_path, header["ElementDataFile"])
            self._file_wrapper = HugeFileWrapper(filename_raw, self._file_factory)
        return self._file_wrapper

    def _get_file_streamer(self):
        if not self._file_streamer:
            header = self._get_header()
            bytes_per_voxel = get_bytes_per_voxel(header["ElementType"])
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

        offset = self.get_linear_byte_offset(self._image_size, self._bytes_per_voxel, start_coords)
        self._file_wrapper.get_handle().seek(offset)
        return self._file_wrapper.get_handle().read(num_voxels_to_read * self._bytes_per_voxel)

    @staticmethod
    def get_linear_byte_offset(image_size, bytes_per_voxel, start_coords):
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

    def __init__(self, name, file_handle_factory):
        self._file_handle_factory = file_handle_factory
        self._filename = name
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
        self._file_handle = self._file_handle_factory.create_file_handle(self._filename, 'rb')

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


def main(args):
    pass


if __name__ == '__main__':
    main(sys.argv[1:])


def get_default_metadata():
    """Return an OrderedDict containing default mhd file metadata"""

    return OrderedDict(
        [('ObjectType', 'Image'), ('NDims', '3'), ('BinaryData', 'True'), ('BinaryDataByteOrderMSB', 'True'),
         ('CompressedData', []), ('CompressedDataSize', []), ('TransformMatrix', []), ('Offset', []),
         ('CenterOfRotation', []), ('AnatomicalOrientation', []), ('ElementSpacing', []), ('DimSize', []),
         ('ElementNumberOfChannels', []), ('ElementSize', []), ('ElementType', 'MET_FLOAT'), ('ElementDataFile', []),
         ('Comment', []), ('SeriesDescription', []), ('AcquisitionDate', []), ('AcquisitionTime', []),
         ('StudyDate', []), ('StudyTime', [])])