# -*- coding: utf-8 -*-

from setuptools import setup, find_packages


with open('README.rst') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

setup(
    name='pyvesseltools',
    version='0.1.0',
    description='Medical image analysis vessel segmentation',
    long_description=readme,
    author='Tom Doel',
    author_email='t.doel@ucl.ac.uk',
    url='https://github.com/giftsurg/pyvesseltools',
    license=license,
    packages=find_packages(exclude=('tests', 'docs'))
)