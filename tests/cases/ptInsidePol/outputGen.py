#!/bin/python

from matplotlib import pyplot as plt

NPoints = int( input() )
wkt = input()

rings = [i.replace(')','').replace('(','') for i in wkt[8:-1].split('),(')]#at this point we're only interested in boobs.

external = []
interior = []

extRing = True
for r in rings:
    newRing = True
    for p in r.split(','):
        x,y = (float(i) for i in p.split(' '))
        if(extRing):
            external += [[x,y]]
        else:
            if(newRing):
                interior.append([[x,y]])
            else:
                interior[-1] += [[x,y]]
        newRing = False
    extRing = False

x = [i[0] for i in external]
y = [i[1] for i in external]
plt.plot(x,y,'-')

x_extent = [min(x),max(x)]
y_extent = [min(y),max(y)]

for i in interior:
    x = [j[0] for j in i]
    y = [j[1] for j in i]
    plt.plot(x,y,'r-')
"""
At this point we have a randomly sampled polygon
"""

points = []
while(True):
    try:
        points.append( [float(i) for i in input()[6:-1].split(' ')] )
    except EOFError:
        break
"""
Read the points
"""

import matplotlib.path as mplPath
import numpy as np

bbPath = mplPath.Path(np.array(external))
containments = bbPath.contains_points(points)

for ring in interior:
    bbPath = mplPath.Path(np.array(ring))
    for i in range(len(points)):
        containments[i] *= not bbPath.contains_point( points[i] )

x = [points[i][0] for i in range(len(points)) if containments[i]]
y = [points[i][1] for i in range(len(points)) if containments[i]]
plt.plot(x,y,'g.')

x = [points[i][0] for i in range(len(points)) if not containments[i]]
y = [points[i][1] for i in range(len(points)) if not containments[i]]
plt.plot(x,y,'r.')

plt.show()

print("\n".join(["1" if i else "0" for i in containments]))

