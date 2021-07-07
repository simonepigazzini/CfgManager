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

import os, shutil, glob
from setuptools.command.install import install

class InstallWrapper(install):
    """Provides a install wrapper for WSGI applications
    to copy additional files for Web servers to use e.g 
    static files. These were packaed using definitions 
    in MANIFEST.in and don't really belong in the 
    Python package."""

    _TARGET_ROOT_PATH = "/usr/local/lib/"

    def run(self):
        # Run this first so the install stops in case 
        # these fail otherwise the Python package is
        # successfully installed
        self._copy_shared_libs(glob.glob(self.build_lib+"/cfgmanager/lib/*"))
        # Run the standard PyPi copy
        install.run(self)

    def _copy_shared_libs(self, libs):
        # Check to see that the required folders exists 
        # Do this first so we don't fail half way
        for lib in libs:
            if not os.access(lib, os.R_OK):
                raise IOError("%s not readable from achive" % 
                              lib)

            # Check to see we can write to the target
            if not os.access(self._TARGET_ROOT_PATH, os.W_OK):
                raise IOError("%s not writeable by user" % 
                    self._TARGET_ROOT_PATH)

        # Clean target and copy files
        for lib in libs:        
            target_path = os.path.join(
                self._TARGET_ROOT_PATH, os.path.basename(lib))

            # If this exists at the target then 
            # remove it to ensure the target is clean
            if os.path.isdir(target_path):
                shutil.rmtree(target_path)

            # Copy the files from the archive
            shutil.copy2(lib, target_path, follow_symlinks=False)

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
    cmdclass={'install': InstallWrapper},
)
