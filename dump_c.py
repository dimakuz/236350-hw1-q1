#!/usr/bin/env python
import sys

with open(sys.argv[1]) as f:
    print 'const char *prologue = %s;\n' % (
        '\t\\\n'.join(
            '"%s\\n"' % l.rstrip('\n')
                .replace('\\', '\\\\')
                .replace('"', '\\"')
            for l in f
        ),
    )
