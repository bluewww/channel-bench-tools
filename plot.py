#!/usr/bin/env python3

import sys
import os
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.ticker import LogLocator, LogFormatterSciNotation, MaxNLocator, MultipleLocator
from matplotlib.widgets import Slider
import tikzplotlib

print(sys.executable)
path_out = None

if os.path.exists(sys.argv[1]):
    path_to_csv = os.path.abspath(sys.argv[1])
else:
    print("Cannot find " + sys.argv[1])
    exit()

linthresh = float(sys.argv[2])

if len(sys.argv) == 4:
    path_out = os.path.abspath(sys.argv[3])

df = pd.read_csv(path_to_csv, delim_whitespace=True).pivot(index='Y', columns='X')

locator = LogLocator(base=2)
formatter = LogFormatterSciNotation(base=2)

fig = plt.figure()#figsize=(1,1.618))
print(fig)

ax = fig.add_subplot()

print(ax)
X = [elem[1] for elem in df.columns]
X.append(X[-1]+1)
Y = list(df.index)
Y.append(Y[-1]+1)

pcm = ax.pcolormesh(X, Y, df, norm=colors.SymLogNorm(linthresh=linthresh, vmin=0, vmax=df.max().max()), rasterized=True)
cbar = fig.colorbar(pcm, ax=ax)
ax.xaxis.set_major_locator(MultipleLocator(base=int (len(X)/8)))
plt.xlabel('Secret')
plt.ylabel('Time (cycles)')
cbar.ax.set_ylabel('Probability')

# Add 5 lines above and underneath
cmap = matplotlib.cm.get_cmap('viridis')
ax.set_facecolor(cmap(0))
plt.ylim(max(min(Y)-5,0),max(Y)+5)

# Square
bottom, top = plt.ylim()
left, right = plt.xlim()
ax.set_aspect((right-left)/(top-bottom))

if path_out is None:
    axthr = plt.axes([0.25, 0, 0.65, 0.03])
    sthr = Slider(axthr, 'threshold', -5, -1)

    def update(val):
        thr = sthr.val
        ax.pcolor(X, Y, df, norm=colors.SymLogNorm(linthresh=10**thr, vmin=0, vmax=df.max().max()))
        fig.canvas.draw_idle()

    sthr.on_changed(update)
    plt.show()
elif path_out.endswith(".tex"):
    plt.tight_layout()
    tikzplotlib.save(path_out, axis_height='\\figH', axis_width='\\figW', dpi=600)
else:
    plt.tight_layout()
    plt.savefig(path_out, dpi=600, transparent=False)
