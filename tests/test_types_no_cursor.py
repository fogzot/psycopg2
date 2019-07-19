#!/usr/bin/env python
#
# types_basic.py - tests for basic types conversions
#
# Copyright (C) 2004-2019 Federico Di Gregorio  <fog@debian.org>
#
# psycopg2 is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# In addition, as a special exception, the copyright holders give
# permission to link this program with the OpenSSL library (or with
# modified versions of OpenSSL that use the same license as OpenSSL),
# and distribute linked combinations including the two.
#
# You must obey the GNU Lesser General Public License in all respects for
# all of the code used other than OpenSSL.
#
# psycopg2 is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.

import string
import ctypes
import decimal
import datetime
import platform

from . import testutils
import unittest
from .testutils import PY2, long, text_type, ConnectingTestCase

import psycopg2
from psycopg2.extensions import AsIs, adapt, register_adapter


class TypesNoCursorTests(ConnectingTestCase):
    """Test that all type conversions are working without a cursor."""

    def testArray(self):
        self.assertEqual([[1, 2], [3, 4]], psycopg2.extensions.INTEGERARRAY('{{1,2},{3,4}}', None))
        self.assertEqual(['one', 'two', 'three'], psycopg2.extensions.STRINGARRAY('{one,two,three}', None))


def test_suite():
    return unittest.TestLoader().loadTestsFromName(__name__)


if __name__ == "__main__":
    unittest.main()
