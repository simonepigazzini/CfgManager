# -*- coding: utf-8 -*-

from __future__ import print_function

import sys

try:
    from skbuild import setup
except ImportError:
    print(
        "Please update pip, you need pip 10 or greater,\n"
        " or you need to install the PEP 518 requirements in pyproject.toml yourself",
        file=sys.stderr,
    )
    raise

from setuptools import find_packages


setup(
    name="cfgmanager",
    version="1.0",
    description="Flexible configuration manager",
    author="Simone Pigazzini",
    license="GPLv3",
    packages=find_packages(where = 'src'),
    package_dir={"": "src"},
    cmake_install_dir="src/cfgmanager",
    include_package_data = True,
)
