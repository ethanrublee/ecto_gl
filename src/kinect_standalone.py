#!/usr/bin/env python
import ecto

from ecto_openni import Capture
from ecto_opencv import imgproc, calib, highgui
from ecto_gl import PointCloudDisplay
capture = Capture('ni device')
display = PointCloudDisplay(window_name='howdy cloud')
verter = highgui.NiConverter('verter')
fps = highgui.FPSDrawer('fps')
plasm = ecto.Plasm()
plasm.connect(
              capture[:] >> display[:],
              capture[:] >> verter[:],
              verter['image'] >> fps[:],
              fps[:] >> highgui.imshow('image display', name='image', waitKey=10)[:],
              verter['depth'] >> highgui.imshow('depth display', name='depth', waitKey= -1)[:],
              )

if __name__ == '__main__':
    sched = ecto.schedulers.Singlethreaded(plasm)
    sched.execute()
