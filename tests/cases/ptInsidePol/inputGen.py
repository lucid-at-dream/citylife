#!/bin/python

from sys import argv
import psycopg2

#query to fetch a random polygon
query = 'select st_astext(way) from planet_osm_polygon where random() < 0.01 AND way_area > 100000 LIMIT 1;'
#connect to database
conn = psycopg2.connect("dbname='portugal' user='magisu' host='localhost'")

interior = []
while(len(interior) == 0):

    cur = conn.cursor()
    cur.execute(query)
    rows = cur.fetchall()

    wkt = rows[0][0]

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

x_extent = [min(x),max(x)]
y_extent = [min(y),max(y)]
"""
At this point we have a randomly sampled polygon
"""

NPoints = int(argv[1])
from random import random

points = [[random()*(x_extent[1]-x_extent[0]) + x_extent[0],
           random()*(y_extent[1]-y_extent[0]) + y_extent[0] ] for i in range(NPoints)]

print(NPoints)
print(wkt)
for p in points:
    print('POINT({0} {1})'.format(p[0], p[1]))




