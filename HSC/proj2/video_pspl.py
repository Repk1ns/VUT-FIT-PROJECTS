# HDMI video capture and replay example for Pynq (PS accelerated by PL)
#
# Author: Lukas Kekely <ikekely@fit.vutbr.cz>
# Date: 1.10.2021

import argparse
import time
import cv2
import numpy
import threading
import math
from overlay.hsc_video import HSCVideoOverlay
from pynq.lib.video import *



# ##############################################################################
# User defined PS functionality
# ##############################################################################

# Insert your specific video data processing here
# You should implement the same functionality as in simulation_pixel_preprocess function
def pixel_preprocess(frame, frame_id):
    # If you do not need to pre-process frame data in the PS, set HDMI_TIE to True at the beggining of main.
    return frame

# Insert your specific per block of frames communication with PL here
# Edit the communication template with your specific PS based control of PL accelerator here
def PSPL_communication(accelerator, last_frames):
    if accelerator.read_write_ready(): # if write operation is ongoing, finish it
        accelerator.write_write_ready_disable()
        if accelerator.read_counter_frames() >= last_frames: # synchronization check = write ended too late for a given block of frames
            print("Frame:", last_frames, "\tWARNING missed partialy, HW thread processing too slow!")
        return last_frames
    new_frames = accelerator.read_counter_frames();
    if new_frames < last_frames: # still in the same old block of frames
        return last_frames
    while last_frames+4 <= new_frames: # synchronization check = missed blocks of frames
        print("Frame:", last_frames, "\t WARNING: missed completely, HW thread processing too slow!")
        last_frames += 4
    if not accelerator.read_read_done(): # waiting for read data ready confirmation from PL
        return last_frames
    sb = accelerator.read_sum_before();
    sa = accelerator.read_sum_after();
    v = accelerator.read_values();
    if not v: # histogram empty
        return last_frames
    
    # ##########################################################################
    # Accelerator specific communication called once per every block of frames
    # ##########################################################################
    histogram = accelerator.read_shared_memory()
    assert v == 1280*720*4, "Wrong pixel count for block of frames!"
    assert v == sum(histogram), "Wrong pixel count in histogram!"
    # process shared memory data somehow?
    P = histogram / v
    Pl = 0.0003
    Pu = P.max() / 2
    Pr = Pu - Pl
    cumsum = 0.0
    empty_data = numpy.zeros(256)
    for k in range(256):
        if P[k] < Pl:
            Pwt = 0.0
        elif P[k] > Pu:
            Pwt = Pu
        else:
            Pwt = math.sqrt((P[k] - Pl) / Pr) * Pu
        cumsum += Pwt
        empty_data[k] = cumsum * 255.0

    empty_data /= cumsum
    empty_data = numpy.around(empty_data).astype(numpy.uint8)
    accelerator.write_shared_memory(empty_data)
    accelerator.write_write_ready_enable()
    # ##########################################################################
    
    print( # TODO: finish the print message!
        "Frame:", last_frames,
        "\tMeans:", "{:.3f}".format(sb/v), "{:.3f}".format(sa/v),
        "\tCumSum:", "{:.3f}".format(cumsum), "\tPu:", "{:.3f}".format(Pu*100.0)
    )
    last_frames += 4
    return last_frames



# ##############################################################################
# List of variables shared between hardware thread and main thread
# ##############################################################################
class SharedData:
    def __init__(self):
        self.running = True # flag to control program running state
        self.accelerator = None # object for communication with pixel proc in HW
        self.hw_frames = 0 # hardware frame counter



# ##############################################################################
# Hardware communication processing loop
# ##############################################################################
def hardware_communication(sd):
    last_frames = 4 # start on frame 4
    while(sd.running):
        last_frames = PSPL_communication(sd.accelerator, last_frames)
        time.sleep(0.0001)
    sd.hw_frames = last_frames



# ##############################################################################
# Main program body
# ##############################################################################
def main():
    # TODO: Enable (true) or disable (false) the PS bypass for video data.
    HDMI_TIE = True 
    # Arguments parsing
    parser = argparse.ArgumentParser(description='HDMI video capture and replay for Pynq.')
    parser.add_argument('-i', metavar='PATH', dest='input', action='store', default=None,
                        help='input file with source image or video (default: use HDMI input port)')
    # Argument -o not supported! Output must always go to HDMI out through PL accelerator.
    parser.add_argument('-l', metavar='N', type=int, dest='loops', action='store', default=1,
                        help='loop input image or video N times (default: 1), 0 means forever')
    parser.add_argument('-c', metavar='N', type=int, dest='max_frames', action='store', default=0,
                        help='stop processing prematurely after N frames (default: 0), 0 means no limit')
    args = parser.parse_args()
    if args.loops < 0: # value cleanup
        args.loops = 0
    if args.max_frames < 1:
        args.max_frames = -1
    
    # Input and output preparation
    print("Preparing input and output ...")
    try: 
        base = HSCVideoOverlay("./overlay/hsc_video.bit")
    except Exception as e:
        exit("ERROR: Unable to boot PL - {}".format(e))
    if args.input == None:
        input = base.video.hdmi_in
        try:
            input.configure()
        except Exception as e:
            exit("ERROR: Unable to read HDMI input data! (Disconnected HDMI cable?)")
        width = input.mode.width
        height = input.mode.height
    else:
        input = cv2.VideoCapture(args.input)
        if not input.isOpened():
            exit("ERROR: Unable to read input file data! (File does not exist or have unssuported format?)")
        width = int(input.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(input.get(cv2.CAP_PROP_FRAME_HEIGHT))
    resize = (width != 1280) or (height != 720)
    output = base.video.hdmi_out
    output.configure(VideoMode(1280, 720, 24, 60)) # 720p video, BGR format, 60 FPS
    sd = SharedData()
    sd.running = True
    sd.accelerator = output.pixel_proc
    HW_thread = threading.Thread(target=hardware_communication, args=[sd])
    output.start()
    if args.input == None:
        input.start()
    if resize or args.input != None:
        HDMI_TIE = False # force video data through PS
        print("WARNING: Performance will be negatively influenced by software overhead!")
        print("NOTE: To remove this overhead use HDMI input (no -i) with 1280x720 resolution.")
    if HDMI_TIE:
        input.tie(output) # dirrect PL video link (PS bypass)

    # Main processing loop
    print("Processing video data ...")
    try:
        frames = 0
        HW_thread.start()
        start = time.time()
        while(frames != args.max_frames):
            if HDMI_TIE: # skip PS video data processing when PL bypass is enabled
                time.sleep(0.1)
                continue
            if args.input != None:
                ret, frame = input.read()
            else:
                frame = input.readframe()
                ret = True
            if ret: # new frame
                if resize:
                    frame = cv2.resize(frame, (1280,720))
                frame = pixel_preprocess(frame, frames) # call user specific pre-processing function
                frames += 1
                outframe = output.newframe()
                outframe[:] = frame
                output.writeframe(outframe)
            else: # one playback loop is finished
                args.loops -= 1; 
                if args.loops != 0:  # restart playback
                    input = cv2.VideoCapture(args.input)
                else:  # stop playback
                    break
    except KeyboardInterrupt:
        print("Interrupted! Ending ...")
    sd.running = False
    end = time.time()

    # Resource cleanup
    print("Cleaning up before end ...")
    HW_thread.join()
    output.close()
    if args.input == None:
        input.close()
    else:
        input.release()

    # Print final statistics
    print()
    print(" Processing time:", "{:.3f}".format(end-start), "s")
    if HDMI_TIE:
        print("Processed frames: not relevant (PL bypass enabled)")
        print("Processing speed: not relevant (PL bypass enabled)")
        print("                  not relevant (PL bypass enabled)")
    else:
        print("Processed frames:", frames)
        print("Processing speed:", "{:.3f}".format(frames/(end-start)), "Fps")
        print("                 ", "{:.3f}".format(((frames*1280*720)/(end-start))/1000000.0), "Mpps")
    print(" Hardware frames:", sd.hw_frames)
    print("  Hardware speed:", "{:.3f}".format(sd.hw_frames/(end-start)), "Fps")
    print("                 ", "{:.3f}".format((sd.hw_frames*1280*720/(end-start))/1000000.0), "Mpps")
    print()

# Calling main when script is executed
if __name__ == "__main__":
    main()
