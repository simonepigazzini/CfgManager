from __future__ import division
import sys
import re
import time
import argparse
oldargv = sys.argv[:]
sys.argv = [ '-b-' ]
import ROOT, os
from ROOT import std
ROOT.gROOT.SetBatch(True)
sys.argv = oldargv

ROOT.gSystem.Load("lib/CfgManagerDict.so")

cfg = ROOT.CfgManager("test.cfg")

cfg.Print()

test_string = cfg.GetOpt(std.string)("test.stringa")

print(test_string)
