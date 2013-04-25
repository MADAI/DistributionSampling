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

\author Hal Canary <cs.unc.edu/~hal/>
"""
useage = """
Usage:
    ............
"""
import sys
import parabolic_potential_model
import madai

if __name__ == '__main__':
	madai.GenerateTraining(
		parabolic_potential_model.ParabolicPotentialModel(),
		100, sys.stdout)
