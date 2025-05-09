#!/usr/bin/env python

import subprocess
import sys
import statistics

if len(sys.argv) < 2:
    print("Usage: python script.py <executable>")
    sys.exit(1)

executable = sys.argv[1]

X = []
Y = []
colors = 'rgb'
idx = 0

axis = []

for executable in sys.argv[1:]:

    axis.append(([],[],colors[idx],executable))
    X = axis[-1][0]
    Y = axis[-1][1]
    idx += 1

    for N in range(100, 501, 50):
        M = N

        elapsed_list = []
        memory_list = []

        for i in range(5):

            # Generate input
            with open("/tmp/b_in", "w") as f:
                subprocess.run(["python3", "in_gen.py", str(N), str(M)], stdout=f)

            # Run the command and capture time and memory
            result = subprocess.run(
                ["/usr/bin/time", "--format=%e %M", "bash", "-c", f"{executable} /tmp/b_in &> /dev/null"],
                stderr=subprocess.PIPE,
                text=True
            )

            out = result.stderr.strip()
            if not out:
                print("Error: No output from time command.")
                continue

            try:
                elapsed, memory = out.split()
                elapsed = float(elapsed)
                memory = int(memory)
            except Exception as ex:
                print(out)
                raise ex


            elapsed_list.append(elapsed)
            memory_list.append(memory)

        # Compute stats
        elapsed_mean = statistics.mean(elapsed_list)
        elapsed_std = statistics.stdev(elapsed_list)
        memory_mean = statistics.mean(memory_list)
        memory_std = statistics.stdev(memory_list)

        print(f"N={N} M={M} | time: {elapsed_mean:.4f}s (+/- {elapsed_std:.4f}s) | memory: {memory_mean}KB (+/- {memory_std:.2f} KB)")

        X.append(N)
        Y.append(elapsed_mean)

print(axis)

from matplotlib import pyplot as p

for x,y,c,e in axis:
    p.plot(x,y,c,label=e)
p.legend()
p.show()
