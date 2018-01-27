#!/usr/bin/python
#
# usage:
#
#   convert <infile.wav> <outfile.h>
#
# https://docs.python.org/2/howto/argparse.html
# https://pysoundfile.readthedocs.io/en/0.8.1/
# https://github.com/bmcfee/resampy

import argparse, soundfile, resampy, itertools

parser = argparse.ArgumentParser()
parser.add_argument("infile")
parser.add_argument("outfile")
args = parser.parse_args()

if len(args.infile) <= 0 or len(args.outfile) <= 0:
    print ("You must provide input wav and output header file paths")
    sys.exit(-1)

# Output sample rate (the sample rate of the Arduino processing loop)
outrate = 31250

# Maximum positive and negative level (signed 8 bit)
int8max = 127

# Read input file
indata, inrate = soundfile.read(args.infile)

# Resample input file to drum machine sample rate
convdata = resampy.resample(indata, inrate, outrate)


def findPeak(samples):
    hi = -1
    lo = 1
    for d in convdata:
        hi = max(hi, d)
        lo = min(lo, d)
    return max(abs(hi), abs(lo))

# Normalize
scale = 1./findPeak(convdata)
convdata = [s * scale for s in convdata]

# Scale and drop zeros at end of data
convdata = [int(s * int8max) for s in convdata]
while convdata[-1] == 0:
    convdata.pop()

print (convdata[-1])

# scale and format
lines = ["%4i" % s for s in convdata]

# form lines of 8 items separated by comma
group_size = 8
lines = [line for line in itertools.izip_longest(*(iter(lines),) * group_size)]
# Remove "None" fills from last line in case the number of samples wasn't a
# multiple of the group_size
lines[-1] = [i for i in itertools.takewhile(lambda x: x is not None, lines[-1])]

# Generate line strings with samples separated by commas
lines = ["  %s," % ", ".join(line) for line in lines]

# Assemble file contents with C header and footer
header = "const char sample_XX[] PROGMEM = \n{"
footer = "};"
text = "%s\n%s\n%s\n" % (header, "\n".join(lines), footer)

# Write file
with open(args.outfile, "w") as f:
    f.write (text)

# Print file to terminal for inspection
print (text)



## Other wav file reading and sample rate conversion options (annoying or
# still buggy though)
#
# https://docs.python.org/2/library/wave.html
# https://pypi.python.org/pypi/samplerate
#
# sample rate conversion
# import numpy as np
# samplerate, enum
# ratio = outrate/inrate
# converter = "sinc_best"  # or 'sinc_fastest', ...
# convdata = samplerate.resample(indata, ratio, converter)
