#!/usr/bin/env python
import ecto

from ecto_openni import Capture, ResolutionMode, Device

from ecto_opencv import imgproc, calib, highgui
from ecto_gl import PointCloudDisplay

def xtion_vga(device_n):
    return Capture('ni device', rgb_resolution=ResolutionMode.VGA_RES,
                   depth_resolution=ResolutionMode.VGA_RES,
                   rgb_fps=30, depth_fps=30,
                   device_number=device_n,
                   registration=True,
                   synchronize=True,
                   device=Device.ASUS_XTION_PRO_LIVE
                   )
def kinect_vga(device_n):
    return Capture('ni device', rgb_resolution=ResolutionMode.VGA_RES,
                   depth_resolution=ResolutionMode.VGA_RES,
                   rgb_fps=30, depth_fps=30,
                   device_number=device_n,
                   registration=True,
                   synchronize=False,
                   device=Device.KINECT
                   )
    
device = 1
#capture = xtion_highres(device)
#capture = xtion_vga(device)
capture = kinect_vga(device)
#capture = kinect_highres(device)
display = PointCloudDisplay(window_name='cloud')

verter = highgui.NiConverter('verter')
fps = highgui.FPSDrawer('fps')
plasm = ecto.Plasm()
plasm.connect(
              capture[:] >> display[:],
              capture[:] >> verter[:],
              verter['image'] >> fps[:],
              fps[:] >> highgui.imshow('image display', name='image', waitKey=10)[:],
              )
#ecto.view_plasm(plasm)
if __name__ == '__main__':
    sched = ecto.schedulers.Threadpool(plasm)
    sched.execute()
