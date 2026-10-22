# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the tlRender project.

# This example renders a comparison of two files to images, with no window:
# a tile of the two side by side, and then a wipe swept across them.
#
# The files are given on the command line like:
# * compare.py a.mov b.mov
# * compare.py render.0001.exr v002/render.0001.exr
#
# The images are written to the current directory. On Linux with no
# display, set SDL_VIDEO_DRIVER=offscreen in the environment.

import sys
import time
import feather_tk as ftk
import tlrender as tl

if len(sys.argv) != 3:
    print("Usage: compare.py a b")
    sys.exit(1)

context = ftk.Context()
tl.init(context)

# Compare the two files with a player.
timeline = tl.Timeline(context, ftk.Path(sys.argv[1]))
player = tl.Player(context, timeline)
player.compare = [tl.Timeline(context, ftk.Path(sys.argv[2]))]

# Nothing updates between calls to Context.tick(), and after a seek the
# current frames are the previous ones until the new ones are decoded, so
# wait for both at the time asked for.
t = timeline.timeRange.start_time
player.currentTime = t
while not (len(player.currentVideo) == 2 and
           all(f.time == t for f in player.currentVideo)):
    context.tick()
    time.sleep(0.005)
frames = player.currentVideo

writeSystem = tl.WriteSystem(context)
def write(fileName, image):
    info = tl.IOInfo()
    info.video = [image.info]
    writer = writeSystem.write(ftk.Path(fileName), info)
    writer.writeVideo(t, image)
    writer.finish()
    print("Wrote", fileName)

# Keep one renderer: the same frames rendered again, with other options,
# stay on the GPU instead of being copied in each time.
render = tl.OffscreenRender(context)

options = tl.CompareOptions()
options.compare = tl.Compare.Tile
write("compare_tile.png", render.render(frames, options))

# A number before the extension would make the file name an image sequence,
# so the wipes are lettered.
options.compare = tl.Compare.Wipe
size = ftk.Size2I(960, 540)
for i, letter in enumerate("abcde"):
    options.wipeCenter = ftk.V2F((i + 1) / 6, 0.5)
    write(f"compare_wipe_{letter}.png", render.render(frames, options, size))
