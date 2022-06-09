import argparse

import cv2


def decode_fourcc(cc):
    return "".join([chr((int(cc) >> 8 * i) & 0xFF) for i in range(4)])


def CaptureVideoTests(frame_width: int, frame_height: int, frames_per_second: int, buffer_size: int):
    ID_FRAMEGRABBER = 2

    cap = cv2.VideoCapture(ID_FRAMEGRABBER, cv2.CAP_V4L2)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, buffer_size)
    #cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('Y', 'U', 'Y', 'V'))
    #cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('B', 'G', 'R', '3')) #don't make any effect and leave it as "YUYV"
    #cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('X', 'V', 'I', 'D')) #don't make any effect and leave it as "YUYV"
    cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G')) #  motion-jpeg codec
    cap.set(cv2.CAP_PROP_FPS, frames_per_second)

    cap.set(cv2.CAP_PROP_FRAME_WIDTH, frame_width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, frame_height)

    # Check if the device is opened correctly
    if not cap.isOpened():
        raise IOError("Cannot open video source {}".format(ID_FRAMEGRABBER))

    # print properties of te capture
    fps = cap.get(cv2.CAP_PROP_FPS)
    w = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
    h = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
    fcc = int(cap.get(cv2.CAP_PROP_FOURCC))
    bs = cap.get(cv2.CAP_PROP_BUFFERSIZE)

    print('fps: {}'.format(fps))
    print('resolution: {}x{}'.format(w, h))
    print('mode: {}'.format(decode_fourcc(fcc)))
    print('Buffer size: {}'.format(bs))

    while (True):
        ret, frame = cap.read()
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        if frame is not None:
            cv2.imshow('frame', frame)
        else:
            print('No frame')
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--fW', required=True, help='Specify width of the image', type=int)
    parser.add_argument('--fH', required=True, help='Specify high of the image', type=int)
    parser.add_argument('--FPS', required=True, help='Specify FPS', type=int)
    parser.add_argument('--buffer_size', required=True, help='Specify buffer_size', type=int)
    args = parser.parse_args()

    CaptureVideoTests(args.fW, args.fH, args.FPS, args.buffer_size)

if __name__ == '__main__':
    main()
