from __future__ import division
import sys
import re
import time
import argparse
import ROOT, os
from ROOT import std
ROOT.gROOT.SetBatch(True)

ROOT.gSystem.Load("lib/libCFGMan.so")

cfg = ROOT.CfgManager("test/test.cfg")
cfg.ParseConfigString("test2.addedFromString 'new option from string'")
cfg.ParseConfigString("test2.addedFromString+= work like a charm")
cfg.ParseConfigString("test2.add+= 'added from py' great")

cfg.Print()

test_string = cfg.GetOpt("test.stringa")

print(test_string)
print(cfg.GetOpt("test.stringa", 1))

for opt in cfg.GetOpt["std::vector<std::string>"]("test.newline"):
    print(opt)

sub_cfg = cfg.GetSubCfg("test3.subtest1")
sub_cfg.Print()

