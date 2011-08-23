#!/usr/bin/env python
import ecto

from ecto_openni import Capture
from ecto_opencv import imgproc, calib, highgui
from ecto_gl import PointCloudDisplay
capture = Capture('ni device')
display = PointCloudDisplay(window_name='howdy cloud')
display2 = PointCloudDisplay(window_name='nother cloud')
verter = highgui.NiConverter('verter')
fps = highgui.FPSDrawer('fps')
plasm = ecto.Plasm()
plasm.connect(
              capture[:] >> (display[:],display2[:]),
              capture[:] >> verter[:],
              verter['image'] >> fps[:],
              fps[:] >> highgui.imshow('image display', name='image', waitKey=10)[:],
              )
#ecto.view_plasm(plasm)
if __name__ == '__main__':
    sched = ecto.schedulers.Threadpool(plasm)
    sched.execute()
