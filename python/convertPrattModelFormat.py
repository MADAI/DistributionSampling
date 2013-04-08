#!/usr/bin/env python
# -*- coding: utf-8 -*-
##=========================================================================
##
##  Copyright 2011-2013 The University of North Carolina at Chapel Hill
##  All rights reserved.
##
##  Licensed under the MADAI Software License. You may obtain a copy of
##  this license at
##
##         https://madai-public.cs.unc.edu/software/license/
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
##=========================================================================

"""
This converts the "Pratt Format" (to be fully documented by Scott
Pratt at some point) into the "Emulator Training Format" (to be fully
documented by Hal Canary at some point).

\author Hal Canary <cs.unc.edu/~hal/>
"""
useage = """
Usage:
    convertPrattModelFormat DIRECTORY_NAME > TRAINING.dat
"""

import sys
import madai

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print useage
        exit(1)
    ( X, Y, UncertaintyScales, Parameters, OutputNames
      ) = madai.ReadDirectoryModelFormat(sys.argv[1])
    if len(X) == 0:
        print 'some error'
        exit(1)
    madai.PrintEmulatorFormat(
        sys.stdout,X,Y,Parameters,OutputNames,UncertaintyScales,[sys.argv[1]])
