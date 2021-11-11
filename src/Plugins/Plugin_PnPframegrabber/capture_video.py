import cv2

def decode_fourcc(cc):
    return "".join([chr((int(cc) >> 8 * i) & 0xFF) for i in range(4)])

ID_GRABBER = 2
ID_WEBCAM = 2

id = ID_GRABBER
#cap = cv2.VideoCapture(id)
cap = cv2.VideoCapture(id, cv2.CAP_V4L2)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
#cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('Y', 'V', '1', '2'))
#cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('Y', 'U', '1', '2'))
#cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('B', 'G', 'R', '3'))
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
# Tested resolutions
# 640 x 480 @ 30Hz
# 800 x 600 @ 20Hz
# 960 x 540
# 1280 x 720  @ 10 fps
# 1920 x 1080 @ 5 fps
cap.set(cv2.CAP_PROP_FPS, 30)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

# Check if the webcam is opened correctly

if not cap.isOpened():
    raise IOError("Cannot open video source {}".format(id))

# print properties of te capture
fps = cap.get(cv2.CAP_PROP_FPS)
w = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
h = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
fcc = int(cap.get(cv2.CAP_PROP_FOURCC))
bs = cap.get(cv2.CAP_PROP_BUFFERSIZE)

print('fps: {}'.format(fps))
print('resolution: {}x{}'.format(w,h)) # default 640 x 480
print('mode: {}'.format(decode_fourcc(fcc))) # default 640 x 480
print('Buffer size: {}'.format(bs)) # default 640 x 480



while(True):
    ret, frame = cap.read()
    if frame is not None:
        cv2.imshow('frame', frame)
    else:
        print('No frame')
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()
