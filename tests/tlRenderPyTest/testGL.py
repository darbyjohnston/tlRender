# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the tlRender project.

import feather_tk as ftk
import tlrender as tl
import opentimelineio as otio

import os
import tempfile
import time
import unittest

def opaqueImage(width, height, value):
    image = ftk.Image(width, height, ftk.ImageType.RGBA_U8)
    pixels = image.data
    for y in range(height):
        for x in range(width):
            pixels[y, x, 0] = value
            pixels[y, x, 1] = value
            pixels[y, x, 2] = value
            pixels[y, x, 3] = 255
    return image

def frame(image):
    layer = tl.VideoLayer()
    layer.image = image
    out = tl.VideoFrame()
    out.size = image.size
    out.layers = [layer]
    return out

def pixel(image, y, x):
    # A memoryview gives one element at a time, not a row of them.
    data = image.data
    return [data[y, x, c] for c in range(4)]

def near(a, b):
    return all(abs(x - y) <= 1 for x, y in zip(a, b))

@unittest.skipIf(os.environ.get("TLRENDER_TESTS_NO_GL"), "OpenGL is not available")
class OffscreenRenderTest(unittest.TestCase):

    def setUp(self):
        self.context = ftk.Context()
        tl.init(self.context)

    def test_compare(self):
        render = tl.OffscreenRender(self.context)
        a = frame(opaqueImage(8, 4, 200))
        b = frame(opaqueImage(8, 4, 100))

        image = render.render([a])
        self.assertEqual(ftk.Size2I(8, 4), image.size)
        self.assertTrue(near([200, 200, 200, 255], pixel(image, 1, 1)))

        options = tl.CompareOptions()
        options.compare = tl.Compare.Horizontal
        image = render.render([a, b], options)
        self.assertEqual(ftk.Size2I(16, 4), image.size)
        self.assertTrue(near([200, 200, 200, 255], pixel(image, 1, 1)))
        self.assertTrue(near([100, 100, 100, 255], pixel(image, 1, 12)))

        # One above the other: A on top, which shows which way up the
        # picture comes back.
        options = tl.CompareOptions()
        options.compare = tl.Compare.Vertical
        image = render.render([a, b], options)
        self.assertEqual(ftk.Size2I(8, 8), image.size)
        self.assertTrue(near([200, 200, 200, 255], pixel(image, 1, 1)))
        self.assertTrue(near([100, 100, 100, 255], pixel(image, 6, 1)))
        options.compare = tl.Compare.Horizontal

        # A wipe: A on the left of the line, B on the right.
        options.compare = tl.Compare.Wipe
        image = render.render([a, b], options)
        self.assertTrue(near([200, 200, 200, 255], pixel(image, 2, 1)))
        self.assertTrue(near([100, 100, 100, 255], pixel(image, 2, 6)))
        options.compare = tl.Compare.Horizontal

        # Fitted into a size of its own, with nothing drawn around it.
        image = render.render([a, b], options, ftk.Size2I(16, 16))
        self.assertEqual(ftk.Size2I(16, 16), image.size)
        self.assertEqual(0, image.data[0, 1, 3])
        self.assertTrue(near([200, 200, 200, 255], pixel(image, 8, 1)))

        with self.assertRaises(Exception):
            render.render([])

    def test_cache(self):
        # Frames rendered again are kept as textures, from the second render
        # in a row that has them, and told apart by the image, not its
        # pixels: changed in place, an image renders as it was until the
        # cache is cleared, and with no cache it renders as it is.
        render = tl.OffscreenRender(self.context)
        self.assertTrue(render.cacheByteCount > 0)
        image = opaqueImage(8, 4, 200)
        a = frame(image)
        for i in range(2):
            self.assertTrue(near([200, 200, 200, 255], pixel(render.render([a]), 1, 1)))
        image.data[1, 1, 0] = 100
        self.assertTrue(near([200, 200, 200, 255], pixel(render.render([a]), 1, 1)))
        render.clearCache()
        self.assertTrue(near([100, 200, 200, 255], pixel(render.render([a]), 1, 1)))
        self.assertTrue(near([100, 200, 200, 255], pixel(render.render([a]), 1, 1)))
        render.cacheByteCount = 0
        self.assertEqual(0, render.cacheByteCount)
        image.data[1, 1, 0] = 50
        self.assertTrue(near([50, 200, 200, 255], pixel(render.render([a]), 1, 1)))

    def test_player(self):
        # The whole of it: two files, compared by a player, rendered as a
        # tile, and written to a file.
        with tempfile.TemporaryDirectory() as dir:
            writeSystem = tl.WriteSystem(self.context)
            fileNames = []
            for name, value in (("a", 200), ("b", 100)):
                image = opaqueImage(8, 4, value)
                fileName = os.path.join(dir, name + ".png")
                info = tl.IOInfo()
                info.video = [image.info]
                writer = writeSystem.write(ftk.Path(fileName), info)
                writer.writeVideo(otio.opentime.RationalTime(0, 24), image)
                writer.finish()
                fileNames.append(fileName)

            timeline = tl.Timeline(self.context, fileNames[0])
            player = tl.Player(self.context, timeline)
            player.compare = [tl.Timeline(self.context, fileNames[1])]
            start = timeline.timeRange.start_time
            player.currentTime = start
            frames = None
            for i in range(500):
                self.context.tick()
                video = player.currentVideo
                if len(video) == 2 and all(
                    v.time == start and v.layers and v.layers[0].image and
                    v.layers[0].image.valid for v in video):
                    frames = video
                    break
                time.sleep(0.01)
            self.assertIsNotNone(frames)
            self.assertEqual(0, player.droppedFrames)

            # Playing in the direction it already plays in speeds it up,
            # and stopping returns it to normal.
            player.forward()
            player.forward()
            self.assertEqual(2.0, player.speedMult)
            player.stop()
            self.assertEqual(1.0, player.speedMult)

            options = tl.CompareOptions()
            options.compare = tl.Compare.Tile
            image = tl.OffscreenRender(self.context).render(frames, options)
            self.assertEqual(ftk.Size2I(16, 4), image.size)
            self.assertTrue(near([200, 200, 200, 255], pixel(image, 1, 1)))
            self.assertTrue(near([100, 100, 100, 255], pixel(image, 1, 12)))

            fileName = os.path.join(dir, "compare.png")
            info = tl.IOInfo()
            info.video = [image.info]
            writer = writeSystem.write(ftk.Path(fileName), info)
            writer.writeVideo(otio.opentime.RationalTime(0, 24), image)
            writer.finish()
            self.assertTrue(os.path.getsize(fileName) > 0)

if __name__ == '__main__':
    unittest.main()
