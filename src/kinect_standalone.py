#!/usr/bin/env python
import ecto

from ecto_openni import Capture
from ecto_opencv import imgproc, calib, highgui
from ecto_gl import PointCloudDisplay
capture = Capture('ni device')
display = PointCloudDisplay(window_name='howdy cloud')

plasm = ecto.Plasm()
plasm.connect(
              capture[:] >> display[:]
              )

if __name__ == '__main__':
    sched = ecto.schedulers.Singlethreaded(plasm)
    sched.execute()
